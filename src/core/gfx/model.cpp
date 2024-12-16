#include <geodesy/core/gfx/model.h>

#include <assert.h>

#include <iostream>

// Model Loading
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static Assimp::Importer* ModelImporter = nullptr;

/*
				  N[0]
				   |
	 ---------------------------------
	 |               |               |
   N[1]             N[2]             N[3]
			  ---------        -----------
			  |       |        |    |    |
			 N[4]    N[5]     N[6] N[7] N[8]
*/

//tex:
// When I am denoting raw vertices in mesh space for the 3d model, I will
// denote them as $ \vec{v} $ so they are not confused with bone space vertices
// $ \vec{v}^{bs} $. This is to eliminate ambiguity in mathematical symbols which
// many don't care for when describing something. The being said, in the node hierarchy
// it is necessary to map mesh space vertices $ \vec{v} \rightarrow \vec{v}^{bs} $
// so that the node hierarchy and its associated animations can then be applied to 
// the mesh. 

//tex:
// When mapping mesh space vertices to bone space vertices, the offset matrix 
// $ A^{offset} $ provided in the bone structure associated with the mesh instance 
// maps them with the following relationship.
// $$ \vec{v}^{bs} = A^{offset} \vec{v} $$

//tex:
// $$ S = \{ B_{1}, B_{2}, \dots B_{n} \} \quad \quad \forall \; i \in \{ 1, 2, \dots n \} $$
// $$ B = \{ A^{bone}, A^{offset}, w \} $$
// When it comes to each mesh instance, they carry bone structures which 
// designating the vertices they are affecting along with a weight $ w_{i} $ indicating
// how much a bone affects the vertex in question. The thing is for convience
// in the per-vertex shader, only four vertex weights with the largest
// weights are passed into the vertex shader, but to maintain mathematical
// generality, the list can be longer. 
// $$ \vec{v}_{\text{model space}} =  \bigg( \sum_{i} A_{i}^{bone} \cdot A_{i}^{offset} w_{i} \bigg) \vec{v} $$
// If there is no bone structure applied to the mesh instance, then the global transform where the mesh
// instance exists will suffice.
// $$ \vec{v}_{\text{model space}} = A^{\text{mesh instance}} \vec{v} $$

//tex:
// How the entire node hierarchy works is that at every node in the 
// tree there exists a transformation matrix which informs how to transform
// on object in the node's space to its parent's node space. The Root node's
// space is the space of the entire model. The cumulative transform from each
// node is thus the map from the node's local space to the model space of the
// object.


namespace geodesy::core::gfx {

	namespace {

		struct texture_type_database {
			std::string Name;
			std::vector<aiTextureType> Type;
			std::shared_ptr<gcl::image> DefaultTexture;
		};

	}

	// Default values for each texture type as unsigned char arrays
	static const unsigned char DefaultColorData[4] 			= {255, 0, 255, 255}; 		// Magenta (Missing Texture Color)
	static const unsigned char DefaultNormalData[4] 		= {128, 128, 255, 255};		// Up vector (0.5, 0.5, 1.0)
	static const unsigned char DefaultHeightData[4] 		= {0, 0, 0, 0}; 			// No displacement
	static const unsigned char DefaultEmissiveData[4] 		= {0, 0, 0, 0}; 			// No emission
	static const unsigned char DefaultOpacityData[4] 		= {255, 255, 255, 255};  	// Fully opaque
	static const unsigned char DefaultAOData[4] 			= {255, 255, 255, 255}; 	// No occlusion
	static const unsigned char DefaultSpecularData[4] 		= {0, 0, 0, 255}; 			// No specularity
	static const unsigned char DefaultAmbientData[4] 		= {0, 0, 0, 255}; 			// No ambient
	static const unsigned char DefaultShininessData[4] 		= {0, 0, 0, 255}; 			// No shininess
	static const unsigned char DefaultMetallicData[4] 		= {0, 0, 0, 255}; 			// Non-metallic, smooth

   static std::vector<texture_type_database> TextureTypeDatabase = {
		{ "Color", 				{ aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR }, 				std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultColorData), (void*)DefaultColorData) 			},
		{ "Normal", 			{ aiTextureType_NORMALS, aiTextureType_NORMAL_CAMERA }, 			std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultNormalData), (void*)DefaultNormalData) 			},
		{ "Height", 			{ aiTextureType_HEIGHT, aiTextureType_DISPLACEMENT }, 				std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultHeightData), (void*)DefaultHeightData) 			},
		{ "Emissive", 			{ aiTextureType_EMISSIVE, aiTextureType_EMISSION_COLOR }, 			std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultEmissiveData), (void*)DefaultEmissiveData) 		},
		{ "Opacity", 			{ aiTextureType_OPACITY }, 											std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultOpacityData), (void*)DefaultOpacityData) 		},
		{ "AmbientOcclusion", 	{ aiTextureType_LIGHTMAP, aiTextureType_AMBIENT_OCCLUSION }, 		std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultAOData), (void*)DefaultAOData) 					},
		{ "Specular", 			{ aiTextureType_SPECULAR }, 										std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultSpecularData), (void*)DefaultSpecularData) 		},
		{ "AmbientLighting", 	{ aiTextureType_AMBIENT }, 											std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultAmbientData), (void*)DefaultAmbientData) 		},
		{ "Shininess", 			{ aiTextureType_SHININESS }, 										std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultShininessData), (void*)DefaultShininessData) 	},
		{ "MetallicRoughness", 	{ aiTextureType_METALNESS, aiTextureType_DIFFUSE_ROUGHNESS }, 		std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultMetallicData), (void*)DefaultMetallicData) 		}
   };

	static std::string absolute_texture_path(std::string aModelPath, aiMaterial *aMaterial, std::vector<aiTextureType> aTextureTypeList) {
		// This function searches a list of texture types, and returns the first texture path it finds.
		// This is to reduce code duplication when searching for equivalent textures in a material.
		aiString String;
		std::string TexturePath = "";
		for (size_t i = 0; i < aTextureTypeList.size(); i++) {
			if (aMaterial->GetTexture(aTextureTypeList[i], 0, &String) == AI_SUCCESS) {
				// Successfully Found a Texture
				TexturePath = aModelPath + "/" + String.C_Str();
				break;
			}
		}
		return TexturePath;
	}

	static std::shared_ptr<gcl::image> load_texture(io::file::manager* aFileManager, std::string aModelPath, aiMaterial *aMaterial, std::vector<aiTextureType> aTextureTypeList) {
		std::string TexturePath = absolute_texture_path(aModelPath, aMaterial, aTextureTypeList);
		if (TexturePath.length() == 0) return nullptr;
		if (aFileManager != nullptr) {
			// Use file manager to load texture
			return std::dynamic_pointer_cast<gcl::image>(aFileManager->open(TexturePath));
		} 
		else {
			return std::make_shared<gcl::image>(TexturePath);
		}
	}

	static void traverse(const aiScene* aScene, aiNode* aNode) {
		static int TreeDepth = -1;
		TreeDepth += 1;
		bool isBone = false;
		for (int i = 0; i < aScene->mNumMeshes; i++) {
			for (int j = 0; j < aScene->mMeshes[i]->mNumBones; j++) {
				std::string A = aScene->mMeshes[i]->mBones[j]->mName.C_Str();
				std::string B = aNode->mName.C_Str();
				if (A == B) {
					isBone = true;
				}
			}
		}
		for (int i = 0; i < TreeDepth; i++) {
			std::cout << "  ";
		}
		std::cout << "Depth: " << TreeDepth << ", isBone: " << isBone << ", Name: " << aNode->mName.C_Str() << std::endl;
		for (int i = 0; i < aNode->mNumMeshes; i++) {
			for (int j = 0; j < TreeDepth; j++) {
				std::cout << " ";
			}
			std::cout << "Mesh Name: " << aScene->mMeshes[aNode->mMeshes[i]]->mName.C_Str() << std::endl;
		}
		for (int i = 0; i < aNode->mNumChildren; i++) {
			traverse(aScene, aNode->mChildren[i]);
		}
		TreeDepth -= 1;
	}

	bool model::initialize() {
		ModelImporter = new Assimp::Importer();
		return (ModelImporter != nullptr);
	}

	void model::terminate() {
		delete ModelImporter;
		ModelImporter = nullptr;
	}

	model::model() {
		this->Time = 0.0;
		this->BindPoseWeight = 1.0f;
	}

	model::model(std::string aFilePath, file::manager* aFileManager) : file(aFilePath) {
		this->Time = 0.0;
		this->BindPoseWeight = 1.0f;
		if (aFilePath.length() == 0) return;
		const aiScene *Scene = ModelImporter->ReadFile(aFilePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

		for (int i = 0; i < Scene->mNumMeshes; i++) {
			std::cout << "Mesh Name: " << Scene->mMeshes[i]->mName.C_Str() << std::endl;
		}
		std::cout << "--------------- Node Hierarchy --------------------" << std::endl;
		traverse(Scene, Scene->mRootNode);
		std::cout << "--------------- Mesh & Bone --------------------" << std::endl;
		for (int i = 0; i < Scene->mNumMeshes; i++) {
			std::cout << "Mesh Name: " << Scene->mMeshes[i]->mName.C_Str() << std::endl;
			for (int j = 0; j < Scene->mMeshes[i]->mNumBones; j++) {
				std::cout << "\tBone Name: " << Scene->mMeshes[i]->mBones[j]->mName.C_Str() << std::endl;
			}
		}

		// Get Name of Model.
		this->Name = Scene->mName.C_Str();

		// Extract Scene Hiearchy
		this->Hierarchy = node(Scene, Scene->mRootNode);

		// Previously incorrect. Animations will be tied to model and not node hierarchy.
		// Each animation contains a map of node animations which are tied to the node hierarchy.
		this->Animation = std::vector<animation>(Scene->mNumAnimations);
		for (size_t i = 0; i < this->Animation.size(); i++) {
			aiAnimation* RA 		= Scene->mAnimations[i];
			animation& LA 			= this->Animation[i];
			LA.Name					= RA->mName.C_Str();
			LA.Weight				= 0.0f; // Animation Disabled until specified otherwise.
			LA.Duration				= RA->mDuration;
			LA.TicksPerSecond		= RA->mTicksPerSecond;
			// Extract Node Animation Animation Structure.
			for (uint j = 0; j < RA->mNumChannels; j++) {
				// Get Node Animation Data Structure
				aiNodeAnim* RNA 										= RA->mChannels[j];
				std::string NodeName 									= RNA->mNodeName.C_Str();
				this->Animation[i].NodeAnimMap[NodeName] 				= animation::node_anim();
				animation::node_anim& LNA 								= this->Animation[i].NodeAnimMap[NodeName];

				// Initialize Vectors For Key Data
				LNA.PositionKey = std::vector<animation::key<math::vec<float, 3>>>(RNA->mNumPositionKeys);	
				LNA.RotationKey = std::vector<animation::key<math::quaternion<float>>>(RNA->mNumRotationKeys);
				LNA.ScalingKey = std::vector<animation::key<math::vec<float, 3>>>(RNA->mNumScalingKeys);

				// Get Position Keys
				for (uint k = 0; k < RNA->mNumPositionKeys; k++) {
					LNA.PositionKey[k].Time = RNA->mPositionKeys[k].mTime;
					LNA.PositionKey[k].Value = math::vec<float, 3>(
						RNA->mPositionKeys[k].mValue.x,
						RNA->mPositionKeys[k].mValue.y,
						RNA->mPositionKeys[k].mValue.z
					);
				}

				// Get Rotation Keys
				for (uint k = 0; k < RNA->mNumRotationKeys; k++) {
					LNA.RotationKey[k].Time = RNA->mRotationKeys[k].mTime;
					LNA.RotationKey[k].Value = math::quaternion<float>(
						RNA->mRotationKeys[k].mValue.w,
						RNA->mRotationKeys[k].mValue.x,
						RNA->mRotationKeys[k].mValue.y,
						RNA->mRotationKeys[k].mValue.z
					);
				}

				// Finally Get Scaling Keys
				for (uint k = 0; k < RNA->mNumScalingKeys; k++) {
					LNA.ScalingKey[k].Time = RNA->mScalingKeys[k].mTime;
					LNA.ScalingKey[k].Value = math::vec<float, 3>(
						RNA->mScalingKeys[k].mValue.x,
						RNA->mScalingKeys[k].mValue.y,
						RNA->mScalingKeys[k].mValue.z
					);
				}
			}
		}

		this->Mesh = std::vector<std::shared_ptr<mesh>>(Scene->mNumMeshes);
		for (size_t i = 0; i < this->Mesh.size(); i++) {
			// Load Raw Vertex Data (Gross, optimize later)
			std::vector<mesh::vertex> VertexData(Scene->mMeshes[i]->mNumVertices);
			for (size_t j = 0; j < VertexData.size(); j++) {
				if (Scene->mMeshes[i]->HasPositions()) {
					VertexData[j].Position = math::vec<float, 3>(
						Scene->mMeshes[i]->mVertices[j].x,
						Scene->mMeshes[i]->mVertices[j].y,
						Scene->mMeshes[i]->mVertices[j].z
					);
				}
				if (Scene->mMeshes[i]->HasNormals()) {
					VertexData[j].Normal = math::vec<float, 3>(
						Scene->mMeshes[i]->mNormals[j].x,
						Scene->mMeshes[i]->mNormals[j].y,
						Scene->mMeshes[i]->mNormals[j].z
					);
				}
				if (Scene->mMeshes[i]->HasTangentsAndBitangents()) {
					VertexData[j].Tangent = math::vec<float, 3>(
						Scene->mMeshes[i]->mTangents[j].x,
						Scene->mMeshes[i]->mTangents[j].y,
						Scene->mMeshes[i]->mTangents[j].z
					);
					VertexData[j].Bitangent = math::vec<float, 3>(
						Scene->mMeshes[i]->mBitangents[j].x,
						Scene->mMeshes[i]->mBitangents[j].y,
						Scene->mMeshes[i]->mBitangents[j].z
					);
				}

				// -------------------- Texturing & Coloring -------------------- //

				// TODO: Support multiple textures
				// Take only the first element of the Texture Coordinate Array.
				for (int k = 0; k < 1 /* AI_MAX_NUMBER_OF_TEXTURECOORDS */; k++) {
					if (Scene->mMeshes[i]->HasTextureCoords(k)) {
						VertexData[j].TextureCoordinate = math::vec<float, 3>(
							Scene->mMeshes[i]->mTextureCoords[k][j].x,
							Scene->mMeshes[i]->mTextureCoords[k][j].y,
							Scene->mMeshes[i]->mTextureCoords[k][j].z
						);
					}
				}
				// Take an average of all the Colors associated with Vertex.
				for (int k = 0; k < 1 /*AI_MAX_NUMBER_OF_COLOR_SETS*/; k++) {
					if (Scene->mMeshes[i]->HasVertexColors(k)) {
						VertexData[j].Color += math::vec<float, 4>(
							Scene->mMeshes[i]->mColors[k][j].r,
							Scene->mMeshes[i]->mColors[k][j].g,
							Scene->mMeshes[i]->mColors[k][j].b,
							Scene->mMeshes[i]->mColors[k][j].a
						);
					}
				}
				// VertexData[j].Color /= AI_MAX_NUMBER_OF_COLOR_SETS;
			}

			// Load Index Data (Used for primitive rendering)
			mesh::topology TopologyData;
			if (Scene->mMeshes[i]->mNumVertices <= (1 << 16)) {
				TopologyData.Data16 = std::vector<ushort>(Scene->mMeshes[i]->mNumFaces * 3);
			}
			else {
				TopologyData.Data32 = std::vector<uint>(Scene->mMeshes[i]->mNumFaces * 3);
			}
			for (size_t j = 0; j < Scene->mMeshes[i]->mNumFaces; j++) {
				if (Scene->mMeshes[i]->mNumVertices <= (1 << 16)) {
					TopologyData.Data16[3*j + 0] = (ushort)Scene->mMeshes[i]->mFaces[j].mIndices[0];
					TopologyData.Data16[3*j + 1] = (ushort)Scene->mMeshes[i]->mFaces[j].mIndices[1];
					TopologyData.Data16[3*j + 2] = (ushort)Scene->mMeshes[i]->mFaces[j].mIndices[2];
				}
				else {
					TopologyData.Data16[3*j + 0] = (uint)Scene->mMeshes[i]->mFaces[j].mIndices[0];
					TopologyData.Data16[3*j + 1] = (uint)Scene->mMeshes[i]->mFaces[j].mIndices[1];
					TopologyData.Data16[3*j + 2] = (uint)Scene->mMeshes[i]->mFaces[j].mIndices[2];
				}
			}

			// Create Mesh Object
			this->Mesh[i] = std::make_shared<mesh>(nullptr, VertexData, TopologyData);
			this->Mesh[i]->Name = Scene->mMeshes[i]->mName.C_Str();
		}

		// What is the point of a material? Material describes the material
		// of a surface. A material has qualities to it that affects how it
		// is seen.
		this->Material = std::vector<std::shared_ptr<material>>(Scene->mNumMaterials);
		for (size_t i = 0; i < Scene->mNumMaterials; i++) {
			aiMaterial *Mat = Scene->mMaterials[i];
			// Create New Material Object.
			this->Material[i] = std::make_shared<material>();
			// Get Material Name
			this->Material[i]->Name = Mat->GetName().C_Str();
			// Get Material Properties.
			{
				// It seems to be there are two rendering systems which materials are 
				// designed to accommodate, the first  is legacy Blinn-Phong and the second
				// is the more modern PBR system. Some data is redundant between the two,
				// but each system has data components that are not used by the other
				// rendering system.

				// Load values from Assimp Material on stack memory, and then convert to internal format.

				aiShadingMode shadingModel;
				aiColor3D diffuseColor;
				aiColor3D emissiveColor;
				aiColor3D ambientColor;
				aiColor3D specularColor;
				float opacity = 1.0f;
				float RefrationIndex = 1.0f;
				float shininess = 0.0f;
				float metallic = 0.0f;
				float roughness = 0.5f;

				// Get shading model.
				Mat->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

				// Base colors
				Mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
				Mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
				Mat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
				Mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);

				// Surface properties
				Mat->Get(AI_MATKEY_OPACITY, opacity);
				Mat->Get(AI_MATKEY_REFRACTI, RefrationIndex);
				Mat->Get(AI_MATKEY_SHININESS, shininess);
				Mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
				Mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

				// Convert to geodesy internal format.
				this->Material[i]->Color = math::vec<float, 3>(diffuseColor.r, diffuseColor.g, diffuseColor.b);
				this->Material[i]->Emissive = math::vec<float, 3>(emissiveColor.r, emissiveColor.g, emissiveColor.b);
				this->Material[i]->Ambient = math::vec<float, 3>(ambientColor.r, ambientColor.g, ambientColor.b);
				this->Material[i]->Specular = math::vec<float, 3>(specularColor.r, specularColor.g, specularColor.b);
				this->Material[i]->Opacity = opacity;
				
				this->Material[i]->Shininess = shininess;
				this->Material[i]->Metallic = metallic;
				this->Material[i]->Roughness = roughness;
			}
			// Get Material Textures.
			for (size_t j = 0; j < TextureTypeDatabase.size(); j++) {
				std::shared_ptr<gcl::image> LoadedTexture = load_texture(aFileManager, this->Directory, Mat, TextureTypeDatabase[j].Type);
				if (LoadedTexture != nullptr) {
					// Texture Loaded Successfully for model.
					this->Material[i]->Texture[TextureTypeDatabase[j].Name] = LoadedTexture;
				}
				else {
					// Texture does not exist, load default texture.
					this->Material[i]->Texture[TextureTypeDatabase[j].Name] = TextureTypeDatabase[j].DefaultTexture;
					if ((TextureTypeDatabase[j].Name == "Color") && (!(this->Material[i]->Color == math::vec<float, 3>(0.0f, 0.0f, 0.0f)))) {
						// Use material values for color texture.
						this->Material[i]->MaterialColorWeight = 1.0f;
					}
				}
			}
		}

		// TODO: Implement direct texture loader later.
		// this->Texture = std::vector<std::shared_ptr<gcl::image>>(Scene->mNumTextures);

		ModelImporter->FreeScene();
	}

	model::model(std::shared_ptr<gcl::context> aContext, std::shared_ptr<model> aModel, gcl::image::create_info aCreateInfo) : model() {
		this->Name = aModel->Name;
		this->Hierarchy = node(aContext, aModel->Hierarchy);
		this->Time = aModel->Time;

		this->Context = aContext;
		// Load Meshes into device memory represntation.
		this->Mesh = std::vector<std::shared_ptr<gfx::mesh>>(aModel->Mesh.size());
		for (std::size_t i = 0; i < aModel->Mesh.size(); i++) {
			this->Mesh[i] = std::make_shared<gfx::mesh>(aContext, aModel->Mesh[i]);
		}

		this->Material = std::vector<std::shared_ptr<gfx::material>>(aModel->Material.size());
		for (std::size_t i = 0; i < aModel->Material.size(); i++) {
			this->Material[i] = std::make_shared<gfx::material>(aContext, aCreateInfo, aModel->Material[i]);
		}

		this->Texture = std::vector<std::shared_ptr<gcl::image>>(aModel->Texture.size());
		for (std::size_t i = 0; i < aModel->Texture.size(); i++) {
			this->Texture[i] = std::make_shared<gcl::image>(aContext, aCreateInfo, aModel->Texture[i]);
		}

	}

	model::~model() {

	}

	void model::update(double aDeltaTime) {
		Time += aDeltaTime;
		// Choose animation here.
		this->Hierarchy.update(this->Time);
	}

}

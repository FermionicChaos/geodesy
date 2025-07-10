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

	// static void traverse(const aiScene* aScene, aiNode* aNode) {
	// 	static int TreeDepth = -1;
	// 	TreeDepth += 1;
	// 	bool isBone = false;
	// 	for (int i = 0; i < aScene->mNumMeshes; i++) {
	// 		for (int j = 0; j < aScene->mMeshes[i]->mNumBones; j++) {
	// 			std::string A = aScene->mMeshes[i]->mBones[j]->mName.C_Str();
	// 			std::string B = aNode->mName.C_Str();
	// 			if (A == B) {
	// 				isBone = true;
	// 			}
	// 		}
	// 	}
	// 	for (int i = 0; i < TreeDepth; i++) {
	// 		std::cout << "  ";
	// 	}
	// 	std::cout << "Depth: " << TreeDepth << ", isBone: " << isBone << ", Name: " << aNode->mName.C_Str() << std::endl;
	// 	for (int i = 0; i < aNode->mNumMeshes; i++) {
	// 		for (int j = 0; j < TreeDepth; j++) {
	// 			std::cout << " ";
	// 		}
	// 		std::cout << "Mesh Name: " << aScene->mMeshes[aNode->mMeshes[i]]->mName.C_Str() << std::endl;
	// 	}
	// 	for (int i = 0; i < aNode->mNumChildren; i++) {
	// 		traverse(aScene, aNode->mChildren[i]);
	// 	}
	// 	TreeDepth -= 1;
	// }

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
	}

	model::model(std::string aFilePath, file::manager* aFileManager) : file(aFilePath) {
		this->Time = 0.0;
		if (aFilePath.length() == 0) return;
		const aiScene *Scene = ModelImporter->ReadFile(aFilePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

		// for (int i = 0; i < Scene->mNumMeshes; i++) {
		// 	std::cout << "Mesh Name: " << Scene->mMeshes[i]->mName.C_Str() << std::endl;
		// }
		// std::cout << "--------------- Node Hierarchy --------------------" << std::endl;
		// traverse(Scene, Scene->mRootNode);
		// std::cout << "--------------- Mesh & Bone --------------------" << std::endl;
		// for (int i = 0; i < Scene->mNumMeshes; i++) {
		// 	std::cout << "Mesh Name: " << Scene->mMeshes[i]->mName.C_Str() << std::endl;
		// 	for (int j = 0; j < Scene->mMeshes[i]->mNumBones; j++) {
		// 		std::cout << "\tBone Name: " << Scene->mMeshes[i]->mBones[j]->mName.C_Str() << std::endl;
		// 	}
		// }

		// Get Name of Model.
		this->Name = Scene->mName.C_Str();

		// Check if root node has meshes, and choose node constructor.
		this->Hierarchy = std::shared_ptr<gfx::node>(new gfx::node(Scene, Scene->mRootNode));

		// Load animation tracks.
		this->Animation = std::vector<phys::animation>(Scene->mNumAnimations);
		for (size_t i = 0; i < this->Animation.size(); i++) {
			this->Animation[i] = phys::animation(Scene->mAnimations[i]);
		}

		// Cleaner way to load meshes.
		this->Mesh = std::vector<std::shared_ptr<mesh>>(Scene->mNumMeshes);
		for (size_t i = 0; i < this->Mesh.size(); i++) {
			this->Mesh[i] = std::shared_ptr<mesh>(new mesh(Scene->mMeshes[i]));
		}

		// Load in materials for the model.
		this->Material = std::vector<std::shared_ptr<material>>(Scene->mNumMaterials);
		for (size_t i = 0; i < Scene->mNumMaterials; i++) {
			this->Material[i] = std::shared_ptr<material>(new material(Scene->mMaterials[i], this->Directory, aFileManager));
		}

		// TODO: Implement direct texture loader later.
		// this->Texture = std::vector<std::shared_ptr<gpu::image>>(Scene->mNumTextures);

		// std::vector<node*> LinearHierarchy = this->Hierarchy.linearize();

		// for (node* N : LinearHierarchy) {
		// 	// Print Name of Node.
		// 	std::cout << "Node Name: " << N->Name << std::endl;
		// 	// Print Bind Pose Transformation Matrix
		// 	// std::cout << "Bind Pose Transformation: " << N->Transformation << std::endl;
		// 	// Print Animation Node Override Data.
		// 	for (animation& A : this->Animation) {
		// 		std::cout << "Animation Name: " << A.Name << std::endl;
		// 		std::cout << "Applies To :" << N->Name << std::endl;
		// 		//std::cout << "Animation Node Transform: " << A[N->Name][0.0] << std::endl;
		// 		std::cout << "Bind Pose - Animation Transformation Matrix" << N->Transformation - A[N->Name][0.0] << std::endl;
		// 	}
		// 	// Print Mesh Instances
		// 	// for (mesh::instance& MI : N->MeshInstance) {
		// 	// 	std::cout << "Mesh Instance: " << MI.Index << std::endl;
		// 	// 	std::cout << "Mesh Instance Transform: " << MI.Transform << std::endl;
		// 	// 	std::cout << "Mesh Instance Bone Data: " << std::endl;
		// 	// 	for (mesh::bone& B : MI.Bone) {
		// 	// 		std::cout << "\tBone Name: " << B.Name << std::endl;
		// 	// 		std::cout << "\tBone Transform: " << B.Transform << std::endl;
		// 	// 		std::cout << "\tBone Offset: " << B.Offset << std::endl;
		// 	// 		std::cout << "\tBone Vertex Weights: " << std::endl;
		// 	// 		for (mesh::bone::weight& W : B.Vertex) {
		// 	// 			std::cout << "\t\tVertex ID: " << W.ID << ", Weight: " << W.Weight << std::endl;
		// 	// 		}
		// 	// 	}
		// 	// }
		// 	// Print Animation Transformations at time 0.
		// }

		ModelImporter->FreeScene();
	}

	model::model(std::shared_ptr<gpu::context> aContext, std::shared_ptr<model> aModel, gpu::image::create_info aCreateInfo) : model() {
		this->Name = aModel->Name;
		this->Context = aContext;

		// Create Node Hierarchy for GPU.
		this->Hierarchy = std::shared_ptr<gfx::node>(new gfx::node(aContext, aModel->Hierarchy.get()));

		// Load node animations.
		this->Animation = aModel->Animation;

		// Load meshes into GPU memory.
		this->Mesh = std::vector<std::shared_ptr<gfx::mesh>>(aModel->Mesh.size());
		for (std::size_t i = 0; i < aModel->Mesh.size(); i++) {
			this->Mesh[i] = std::shared_ptr<mesh>(new mesh(aContext, aModel->Mesh[i]));
		}

		// Load materials into GPU memory.
		this->Material = std::vector<std::shared_ptr<gfx::material>>(aModel->Material.size());
		for (std::size_t i = 0; i < aModel->Material.size(); i++) {
			this->Material[i] = std::shared_ptr<material>(new material(aContext, aCreateInfo, aModel->Material[i]));
		}

		// Load textures into GPU memory.
		this->Texture = std::vector<std::shared_ptr<gpu::image>>(aModel->Texture.size());
		for (std::size_t i = 0; i < aModel->Texture.size(); i++) {
			this->Texture[i] = std::shared_ptr<gpu::image>(new gpu::image(aContext, aCreateInfo, aModel->Texture[i]));
		}

	}

	model::~model() {

	}

}

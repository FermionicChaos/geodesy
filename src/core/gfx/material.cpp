#include <geodesy/core/gfx/material.h>

#include <vector>

#include <geodesy/core/gcl/context.h>
#include <geodesy/core/gcl/shader.h>
#include <geodesy/core/gcl/image.h>

// Model Loading
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace geodesy::core::gfx {

	using namespace gcl;

	namespace {

		struct texture_type_database {
			std::string Name;
			std::vector<aiTextureType> Type;
			std::shared_ptr<gcl::image> DefaultTexture;
		};

	}

	// Default values for each texture type as unsigned char arrays
	static const unsigned char DefaultColorData[4] 			= {255, 0, 255, 255}; 		// Magenta (Missing Texture Color)
	static const unsigned char DefaultSpecularData[4] 		= {0, 0, 0, 255}; 			// No specularity
	static const unsigned char DefaultAmbientData[4] 		= {0, 0, 0, 255}; 			// No ambient
	static const unsigned char DefaultEmissiveData[4] 		= {0, 0, 0, 0}; 			// No emission
	static const unsigned char DefaultHeightData[4] 		= {0, 0, 0, 0}; 			// No displacement
	static const unsigned char DefaultNormalData[4] 		= {128, 128, 255, 255};		// Up vector (0.5, 0.5, 1.0)
	static const unsigned char DefaultShininessData[4] 		= {0, 0, 0, 255}; 			// No shininess
	static const unsigned char DefaultOpacityData[4] 		= {255, 255, 255, 255};  	// Fully opaque
	static const unsigned char DefaultAOData[4] 			= {255, 255, 255, 255}; 	// No occlusion
	static const unsigned char DefaultMetallicData[4] 		= {0, 0, 0, 255}; 			// Non-metallic, smooth
	static const unsigned char DefaultRoughnessData[4] 		= {255, 255, 255, 255}; 	// Smooth
	static const unsigned char DefaultSheenData[4] 			= {0, 0, 0, 255}; 			// No sheen
	static const unsigned char DefaultClearCoatData[4] 		= {0, 0, 0, 255}; 			// No clear coat

   static std::vector<texture_type_database> TextureTypeDatabase = {
		{ "Color", 					{ aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR }, 			std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultColorData), (void*)DefaultColorData) 			},
		{ "Specular", 				{ aiTextureType_SPECULAR }, 									std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultSpecularData), (void*)DefaultSpecularData) 		},	
		{ "AmbientLighting" , 		{ aiTextureType_AMBIENT }, 										std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultAmbientData), (void*)DefaultAmbientData) 		},
		{ "Emissive", 				{ aiTextureType_EMISSIVE, aiTextureType_EMISSION_COLOR }, 		std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultEmissiveData), (void*)DefaultEmissiveData) 		},
		{ "Height", 				{ aiTextureType_HEIGHT, aiTextureType_DISPLACEMENT }, 			std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultHeightData), (void*)DefaultHeightData) 			},
		{ "Normal", 				{ aiTextureType_NORMALS /*, aiTextureType_NORMAL_CAMERA*/ }, 	std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultNormalData), (void*)DefaultNormalData) 			},
		{ "Shininess", 				{ aiTextureType_SHININESS }, 									std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultShininessData), (void*)DefaultShininessData) 	},
		{ "Opacity", 				{ aiTextureType_OPACITY, aiTextureType_TRANSMISSION }, 			std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultOpacityData), (void*)DefaultOpacityData) 		},
		{ "AmbientOcclusion", 		{ aiTextureType_LIGHTMAP, aiTextureType_AMBIENT_OCCLUSION }, 	std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultAOData), (void*)DefaultAOData) 					},
		// { "Reflection", 			{ aiTextureType_REFLECTION }, 									nullptr },
		{ "Metallic", 				{ aiTextureType_METALNESS }, 									std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultMetallicData), (void*)DefaultMetallicData) 		},
		{ "Roughness", 				{ aiTextureType_DIFFUSE_ROUGHNESS }, 							std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultRoughnessData), (void*)DefaultRoughnessData) 	},
		{ "Sheen", 					{ aiTextureType_SHEEN }, 										std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultSheenData), (void*)DefaultSheenData) 			},
		{ "ClearCoat", 				{ aiTextureType_CLEARCOAT }, 									std::make_shared<gcl::image>(gcl::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultClearCoatData), (void*)DefaultClearCoatData) 	}
   };
   // ! TODO: Metallic, Roughness, and AO are some times packed into the same file, so find a way to work with this data backend and shader wise.

	static std::string absolute_texture_path(std::string aModelPath, const aiMaterial *aMaterial, std::vector<aiTextureType> aTextureTypeList) {
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

	static std::shared_ptr<gcl::image> load_texture(io::file::manager* aFileManager, std::string aModelPath, const aiMaterial *aMaterial, std::vector<aiTextureType> aTextureTypeList) {
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

	material::uniform_data::uniform_data() {
		this->Color                     = { 1.0f, 0.0f, 1.0f };
		this->Emissive                  = { 0.0f, 0.0f, 0.0f };
		this->Ambient                   = { 0.0f, 0.0f, 0.0f };
		this->Specular                  = { 0.0f, 0.0f, 0.0f };
		this->Opacity                   = 1.0f;
		this->RefractionIndex           = 1.0f;
		this->Shininess                 = 0.0f;
		this->Metallic                  = 0.0f;
		this->Roughness                 = 0.0f;
		this->VertexColorWeight         = 0.0f;
		this->MaterialColorWeight       = 0.0f;
		this->ParallaxScale             = 1.0f;
		this->ParallaxIterationCount    = 16;
	}

	material::uniform_data::uniform_data(const material* aMaterial) : uniform_data() {
		*this = aMaterial->UniformData;
	}

	material::material() {
		this->Name                      = "";
		this->RenderingSystem           = UNKNOWN;
		this->Transparency              = OPAQUE;
	}

	material::material(const aiMaterial* Mat, std::string aDirectory, io::file::manager* aFileManager) : material() {
		// Get Material Name
		this->Name = Mat->GetName().C_Str();
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
			this->UniformData.Color = math::vec<float, 3>(diffuseColor.r, diffuseColor.g, diffuseColor.b);
			this->UniformData.Emissive = math::vec<float, 3>(emissiveColor.r, emissiveColor.g, emissiveColor.b);
			this->UniformData.Ambient = math::vec<float, 3>(ambientColor.r, ambientColor.g, ambientColor.b);
			this->UniformData.Specular = math::vec<float, 3>(specularColor.r, specularColor.g, specularColor.b);
			this->UniformData.Opacity = opacity;
			
			this->UniformData.Shininess = shininess;
			this->UniformData.Metallic = metallic;
			this->UniformData.Roughness = roughness;
		}
		// Get Material Textures.
		for (size_t j = 0; j < TextureTypeDatabase.size(); j++) {
			std::shared_ptr<gcl::image> LoadedTexture = load_texture(aFileManager, aDirectory, Mat, TextureTypeDatabase[j].Type);
			if (LoadedTexture != nullptr) {
				// Texture Loaded Successfully for model.
				this->Texture[TextureTypeDatabase[j].Name] = LoadedTexture;
			}
			else {
				// Texture does not exist, load default texture.
				this->Texture[TextureTypeDatabase[j].Name] = TextureTypeDatabase[j].DefaultTexture;
				// TODO: This is a hack to get the material color texture to work, fix later.
				if ((TextureTypeDatabase[j].Name == "Color") && (!(this->UniformData.Color == math::vec<float, 3>(0.0f, 0.0f, 0.0f)))) {
					// Use material values for color texture.
					this->UniformData.MaterialColorWeight = 1.0f;
				}
			}
		}
	}

	material::material(std::shared_ptr<gcl::context> aContext, gcl::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial) : material() {
		this->Name              = aMaterial->Name;
		this->RenderingSystem   = aMaterial->RenderingSystem;
		this->Transparency      = aMaterial->Transparency;

		// Create GPU Uniform Buffer for material properties.
		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		uniform_data MaterialData = uniform_data(aMaterial.get());
		this->UniformBuffer = aContext->create_buffer(UBCI, sizeof(uniform_data), &MaterialData);
		this->UniformBuffer->map_memory(0, sizeof(uniform_data));

		// Copy over and create GPU instance textures.
		for (auto& Texture : aMaterial->Texture) {
			this->Texture[Texture.first] = std::make_shared<gcl::image>(aContext, aCreateInfo, Texture.second);
		}
	}

	material::~material() {}

	void material::update(double aDeltaTime) {
		// Update Material Properties
		// material_data MaterialData = material_data(this);
		// memcpy(this->UniformBuffer->Ptr, &MaterialData, sizeof(material_data));
	}

}

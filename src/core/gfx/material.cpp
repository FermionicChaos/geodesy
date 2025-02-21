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
		// Default Rendering Properties
		this->RenderingSystem 					= 0; 						// Default rendering system (can be defined later)
		this->Transparency 						= 0; 						// No transparency by default
		this->VertexColorWeight 				= 0.0f;
		this->TextureColorWeight 				= 0.0f;
		this->ColorWeight 						= 1.0f; 					// Default to solid color
		this->Color 							= {1.0f, 0.0f, 1.0f}; 		// magenta (missing texture color)

		this->TextureSpecularWeight 			= 0.0f;
		this->SpecularWeight 					= 1.0f;
		this->Specular 							= {0.04f, 0.04f, 0.04f}; 	// Default dielectric specular

		this->TextureAmbientWeight 				= 0.0f;
		this->AmbientWeight 					= 1.0f;
		this->AmbientLighting 					= {0.0f, 0.0f, 0.0f}; 		// No ambient contribution

		this->TextureEmissiveWeight 			= 0.0f;
		this->EmissiveWeight 					= 1.0f;
		this->Emissive 							= {0.0f, 0.0f, 0.0f}; 		// No emissive light

		this->TextureShininessWeight 			= 0.0f;
		this->ShininessWeight 					= 1.0f;
		this->Shininess 						= 32.0f; 					// Default for Blinn-Phong

		this->TextureOpacityWeight 				= 0.0f;
		this->OpacityWeight 					= 1.0f;
		this->Opacity 							= 1.0f; 					// Fully opaque

		this->VertexNormalWeight 				= 1.0f; 					// Use vertex normals by default
		this->TextureNormalWeight 				= 0.0f; 					// No normal map influence unless loaded

		this->TextureAmbientOcclusionWeight 	= 0.0f;
		this->AmbientOcclusionWeight 			= 1.0f;
		this->AmbientOcclusion 					= 1.0f; 					// Full ambient light (no occlusion)

		this->TextureReflectionWeight 			= 0.0f;
		this->ReflectionWeight 					= 1.0f;
		this->Reflection 						= 0.0f; 					// No reflections by default

		this->TextureMetallicWeight 			= 0.0f;
		this->MetallicWeight 					= 1.0f;
		this->Metallic 							= 0.0f; 					// Non-metallic default

		this->TextureRoughnessWeight 			= 0.0f;
		this->RoughnessWeight 					= 1.0f;
		this->Roughness 						= 1.0f; 					// Fully rough surface
		
		this->TextureSheenWeight 				= 0.0f;
		this->SheenWeight 						= 1.0f;
		this->SheenColor 						= {0.0f, 0.0f, 0.0f}; 		// No sheen effect
		this->SheenRoughness 					= 0.5f; 					// Mid-range sheen roughness

		this->TextureClearCoatWeight 			= 0.0f;
		this->ClearCoatWeight 					= 1.0f;
		this->ClearCoat 						= 0.0f; 					// No clear coat by default
		this->ClearCoatRoughness 				= 0.0f; 					// Fully smooth clear coat

		this->RefractionIndex 					= 1.0f; 					// Air refraction index

		this->Anisotropy 						= 0.0f; 					// No anisotropy by default
		this->AnisotropyDirection 				= {1.0f, 0.0f, 0.0f}; 		// Default anisotropy direction (X-axis)

		this->SubsurfaceScattering 				= 0.0f; 					// No SSS effect
		
		this->ParallaxScale 					= 0.0f; 					// No parallax effect
		this->ParallaxIterationCount 			= 0; 						// No parallax iterations
	}

	material::material() {
		this->Name = "";
	}

	material::material(const aiMaterial* Mat, std::string aDirectory, io::file::manager* aFileManager) : material() {
		// Get Material Name
		this->Name = Mat->GetName().C_Str();
		// Get Material Properties.
		// Load Material Properties
		{
			aiShadingMode shadingModel;
			aiColor3D diffuseColor, emissiveColor, ambientColor, specularColor, sheenColor;
			float opacity = 1.0f, refractionIndex = 1.0f, shininess = 0.0f;
			float metallic = 0.0f, roughness = 0.5f, sheenRoughness = 0.5f, transmission = 0.0f;
			float clearCoat = 0.0f, clearCoatRoughness = 0.0f, anisotropy = 0.0f;
			aiColor3D anisotropyDirection(1.0f, 0.0f, 0.0f); // Default along X-axis
	
			// Get shading model
			Mat->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
	
			// Base colors
			Mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
			Mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
			Mat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
			Mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
			Mat->Get(AI_MATKEY_SHEEN_COLOR_FACTOR, sheenColor);
	
			// Surface properties
			Mat->Get(AI_MATKEY_OPACITY, opacity);
			Mat->Get(AI_MATKEY_REFRACTI, refractionIndex);
			Mat->Get(AI_MATKEY_SHININESS, shininess);
			Mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
			Mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
			Mat->Get(AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, sheenRoughness);
			Mat->Get(AI_MATKEY_TRANSMISSION_FACTOR, transmission);
			Mat->Get(AI_MATKEY_CLEARCOAT_FACTOR, clearCoat);
			Mat->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, clearCoatRoughness);
			Mat->Get(AI_MATKEY_ANISOTROPY_FACTOR, anisotropy);
			// Mat->Get(AI_MATKEY_ANISOTROPY_DIRECTION, anisotropyDirection); // Doesn't Exist in Assimp
	
			// Convert to Geodesy Engine's internal format
			this->UniformData.Color = math::vec<float, 3>(diffuseColor.r, diffuseColor.g, diffuseColor.b);
			this->UniformData.Emissive = math::vec<float, 3>(emissiveColor.r, emissiveColor.g, emissiveColor.b);
			this->UniformData.AmbientLighting = math::vec<float, 3>(ambientColor.r, ambientColor.g, ambientColor.b);
			this->UniformData.Specular = math::vec<float, 3>(specularColor.r, specularColor.g, specularColor.b);
			this->UniformData.Opacity = opacity;
			this->UniformData.RefractionIndex = refractionIndex;
			this->UniformData.Shininess = shininess;
			this->UniformData.Metallic = metallic;
			this->UniformData.Roughness = roughness;
			this->UniformData.SheenColor = math::vec<float, 3>(sheenColor.r, sheenColor.g, sheenColor.b);
			this->UniformData.SheenRoughness = sheenRoughness;
			// this->UniformData.Transmission = transmission; // Removed
			this->UniformData.ClearCoat = clearCoat;
			this->UniformData.ClearCoatRoughness = clearCoatRoughness;
			this->UniformData.Anisotropy = anisotropy;
			// this->UniformData.AnisotropyDirection = math::vec<float, 3>(anisotropyDirection.r, anisotropyDirection.g, anisotropyDirection.b); // Removed
		}

		// Get Material Textures.
		for (const auto& TextureType : TextureTypeDatabase) {
			std::shared_ptr<gcl::image> LoadedTexture = load_texture(aFileManager, aDirectory, Mat, TextureType.Type);
			if (LoadedTexture != nullptr) {
				// Texture Loaded Successfully
				this->Texture[TextureType.Name] = LoadedTexture;

				// Since texture exists, override the default material constant weights for each loaded texture.
				// ! This is ugly, change later.
				if (TextureType.Name == "Color") {
					this->UniformData.VertexColorWeight = 0.0f;
					this->UniformData.TextureColorWeight = 1.0f;
					this->UniformData.ColorWeight = 0.0f;
				}
				if (TextureType.Name == "Specular") {
					this->UniformData.TextureSpecularWeight = 1.0f;
					this->UniformData.SpecularWeight = 0.0f;
				}
				if (TextureType.Name == "AmbientLighting") {
					this->UniformData.TextureAmbientWeight = 1.0f;
					this->UniformData.AmbientWeight = 0.0f;
				}
				if (TextureType.Name == "Emissive") {
					this->UniformData.TextureEmissiveWeight = 1.0f;
					this->UniformData.EmissiveWeight = 0.0f;
				}
				if (TextureType.Name == "Shininess") {
					this->UniformData.TextureShininessWeight = 1.0f;
					this->UniformData.ShininessWeight = 0.0f;
				}
				if (TextureType.Name == "Opacity") {
					this->UniformData.TextureOpacityWeight = 1.0f;
					this->UniformData.OpacityWeight = 0.0f;
				}
				if (TextureType.Name == "Normal") {
					this->UniformData.VertexNormalWeight = 0.0f;
					this->UniformData.TextureNormalWeight = 1.0f;
				}
				if (TextureType.Name == "AmbientOcclusion") {
					this->UniformData.TextureAmbientOcclusionWeight = 1.0f;
					this->UniformData.AmbientOcclusionWeight = 0.0f;
				}
				if (TextureType.Name == "Reflection") {
					this->UniformData.TextureReflectionWeight = 1.0f;
					this->UniformData.ReflectionWeight = 0.0f;
				}
				if (TextureType.Name == "Metallic") {
					this->UniformData.TextureMetallicWeight = 1.0f;
					this->UniformData.MetallicWeight = 0.0f;
				}
				if (TextureType.Name == "Roughness") {
					this->UniformData.TextureRoughnessWeight = 1.0f;
					this->UniformData.RoughnessWeight = 0.0f;
				}
				if (TextureType.Name == "Sheen") {
					this->UniformData.TextureSheenWeight = 1.0f;
					this->UniformData.SheenWeight = 0.0f;
				}
				if (TextureType.Name == "ClearCoat") {
					this->UniformData.TextureClearCoatWeight = 1.0f;
					this->UniformData.ClearCoatWeight = 0.0f;
				}
			} else {
				// Texture does not exist, load default texture
				this->Texture[TextureType.Name] = TextureType.DefaultTexture;
			}
		}
	}

	material::material(std::shared_ptr<gcl::context> aContext, gcl::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial) : material() {
		this->Name              = aMaterial->Name;
		this->UniformData       = aMaterial->UniformData;

		// Create GPU Uniform Buffer for material properties.
		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		this->UniformBuffer = aContext->create_buffer(UBCI, sizeof(uniform_data), &this->UniformData);
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

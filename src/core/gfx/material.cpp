#include <geodesy/core/gfx/material.h>

#include <vector>

#include <geodesy/core/gpu/context.h>
#include <geodesy/core/gpu/shader.h>
#include <geodesy/core/gpu/image.h>

// Model Loading
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace geodesy::core::gfx {

	using namespace gpu;

	namespace {

		struct texture_type_database {
			std::string Name;
			std::vector<aiTextureType> Type;
			std::shared_ptr<gpu::image> DefaultTexture;
		};

	}

	// Default values for each texture type as unsigned char arrays
	static const unsigned char DefaultColorData[4]              = {255, 0, 255, 255};   // Magenta (Missing Texture Color)
	static const unsigned char DefaultOpacityData[4]            = {255, 255, 255, 255}; // Fully opaque
	static const unsigned char DefaultNormalData[4]             = {128, 128, 255, 255}; // Up vector (0.5, 0.5, 1.0)
	static const unsigned char DefaultHeightData[4]             = {0, 0, 0, 255};       // No displacement
	static const unsigned char DefaultAmbientLightingData[4]    = {0, 0, 0, 255};       // No ambient lighting
	static const unsigned char DefaultEmissiveData[4]           = {0, 0, 0, 255};       // No emission
	static const unsigned char DefaultSpecularData[4]           = {0, 0, 0, 255};       // No specularity
	static const unsigned char DefaultShininessData[4]          = {0, 0, 0, 255};       // No shininess
	static const unsigned char DefaultAmbientOcclusionData[4]   = {255, 255, 255, 255}; // No occlusion (fully lit)
	static const unsigned char DefaultMetallicData[4]           = {0, 0, 0, 255};       // Non-metallic
	static const unsigned char DefaultRoughnessData[4]          = {128, 128, 128, 255}; // FIX: Medium roughness (0.5)
	static const unsigned char DefaultSheenData[4]              = {0, 128, 0, 255};     // FIX: No sheen, medium roughness
	static const unsigned char DefaultClearCoatData[4]          = {0, 32, 0, 255};      // FIX: No clear coat, smooth surface

	static std::vector<texture_type_database> TextureTypeDatabase = {
		{ "Albedo", 				{ aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR }, 			geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultColorData), (void*)DefaultColorData) 								},
		{ "Opacity", 				{ aiTextureType_OPACITY, aiTextureType_TRANSMISSION }, 			geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultOpacityData), (void*)DefaultOpacityData) 							},
		// { "Reflection", 			{ aiTextureType_REFLECTION }, 									nullptr },
		{ "Normal", 				{ aiTextureType_NORMALS /*, aiTextureType_NORMAL_CAMERA*/ }, 	geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultNormalData), (void*)DefaultNormalData) 								},
		{ "Height", 				{ aiTextureType_HEIGHT, aiTextureType_DISPLACEMENT }, 			geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultHeightData), (void*)DefaultHeightData) 								},
		{ "AmbientLighting" , 		{ aiTextureType_AMBIENT }, 										geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultAmbientLightingData), (void*)DefaultAmbientLightingData) 			},
		{ "Emissive", 				{ aiTextureType_EMISSIVE, aiTextureType_EMISSION_COLOR }, 		geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultEmissiveData), (void*)DefaultEmissiveData) 							},
		{ "Specular", 				{ aiTextureType_SPECULAR }, 									geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultSpecularData), (void*)DefaultSpecularData) 							},	
		{ "Shininess", 				{ aiTextureType_SHININESS }, 									geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultShininessData), (void*)DefaultShininessData) 						},
		{ "AmbientOcclusion", 		{ aiTextureType_LIGHTMAP, aiTextureType_AMBIENT_OCCLUSION }, 	geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultAmbientOcclusionData), (void*)DefaultAmbientOcclusionData) 			},
		{ "Metallic", 				{ aiTextureType_METALNESS }, 									geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultMetallicData), (void*)DefaultMetallicData) 							},
		{ "Roughness", 				{ aiTextureType_DIFFUSE_ROUGHNESS }, 							geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultRoughnessData), (void*)DefaultRoughnessData) 						},
		{ "Sheen", 					{ aiTextureType_SHEEN }, 										geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultSheenData), (void*)DefaultSheenData) 								},
		{ "ClearCoat", 				{ aiTextureType_CLEARCOAT }, 									geodesy::make<gpu::image>(gpu::image::format::R8G8B8A8_UNORM, 2, 2, 1, 1, sizeof(DefaultClearCoatData), (void*)DefaultClearCoatData) 						}
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

	static std::shared_ptr<gpu::image> load_texture(io::file::manager* aFileManager, std::string aModelPath, const aiMaterial *aMaterial, std::vector<aiTextureType> aTextureTypeList) {
		std::string TexturePath = absolute_texture_path(aModelPath, aMaterial, aTextureTypeList);
		if (TexturePath.length() == 0) return nullptr;
		if (aFileManager != nullptr) {
			// Use file manager to load texture
			return std::dynamic_pointer_cast<gpu::image>(aFileManager->open(TexturePath));
		} 
		else {
			return geodesy::make<gpu::image>(TexturePath);
		}
	}

	material::uniform_data::uniform_data() {
		this->Transparency 									= material::transparency::OPAQUE;
		this->AlbedoVertexWeight 							= 0.0f;
		this->AlbedoTextureWeight 							= 0.0f;
		this->AlbedoWeight 									= 1.0f;
		this->AlbedoTextureIndex 							= -1;
		this->Albedo 										= { 1.0f, 0.0f, 1.0f }; // Missing Texture Color (Magenta)
		this->OpacityTextureWeight 							= 0.0f;
		this->OpacityWeight 								= 1.0f;
		this->OpacityTextureIndex 							= -1;
		this->Opacity 										= 1.0f;
		this->NormalVertexWeight 							= 1.0f;
		this->NormalTextureWeight 							= 0.0f;
		this->NormalTextureIndex 							= -1;
		this->HeightTextureIndex 							= -1;
		this->HeightScale 									= 1.0f;
		this->HeightStepCount 								= 1;
		this->AmbientLightingTextureWeight 					= 0.0f;
		this->AmbientLightingWeight 						= 0.0f;
		this->AmbientLightingTextureIndex 					= -1;
		this->AmbientLighting 								= { 0.0f, 0.0f, 0.0f };
		this->EmissiveTextureWeight 						= 0.0f;
		this->EmissiveWeight 								= 0.0f;
		this->EmissiveTextureIndex 							= -1;
		this->Emissive 										= { 0.0f, 0.0f, 0.0f };
		this->SpecularTextureWeight 						= 0.0f;
		this->SpecularWeight 								= 0.0f;
		this->SpecularTextureIndex 							= -1;
		this->Specular 										= { 0.0f, 0.0f, 0.0f };
		this->ShininessTextureWeight 						= 0.0f;
		this->ShininessWeight 								= 0.0f;
		this->ShininessTextureIndex 						= -1;
		this->Shininess 									= 0.0f;
		this->AmbientOcclusionTextureWeight 				= 0.0f;
		this->AmbientOcclusionWeight 						= 0.0f;
		this->AmbientOcclusionTextureIndex 					= -1;
		this->AmbientOcclusion 								= 0.0f;
		this->MetallicTextureWeight 						= 0.0f;
		this->MetallicWeight 								= 0.0f;
		this->MetallicTextureIndex 							= -1;
		this->Metallic 										= 0.0f;
		this->RoughnessTextureWeight 						= 0.0f;
		this->RoughnessWeight 								= 0.0f;
		this->RoughnessTextureIndex 						= -1;
		this->Roughness 									= 0.0f;
		this->IORVertexWeight 								= 0.0f;
		this->IORTextureWeight 								= 0.0f;
		this->IORWeight 									= 1.0f;
		this->IORTextureIndex 								= -1;
		this->IOR 											= 1.0f;
		this->SheenColorTextureWeight						= 0.0f;
		this->SheenColorWeight								= 0.0f;
		this->SheenColorTextureIndex						= -1;
		this->SheenColor									= { 0.0f, 0.0f, 0.0f };
		this->SheenIntensityTextureWeight					= 0.0f;
		this->SheenIntensityWeight							= 0.0f;
		this->SheenRoughnessTextureWeight					= 0.0f;
		this->SheenRoughnessWeight							= 0.0f;
		this->SheenMaskTextureIndex							= -1;
		this->SheenRoughness								= 0.0f;
		this->SheenIntensity								= 0.0f;
		this->ClearCoatStrengthTextureWeight				= 0.0f;
		this->ClearCoatStrengthWeight						= 0.0f;
		this->ClearCoatRoughnessTextureWeight				= 0.0f;
		this->ClearCoatRoughnessWeight						= 0.0f;
		this->ClearCoatStrengthRoughnessTextureIndex		= -1;
		this->ClearCoatStrength								= 0.0f;
		this->ClearCoatRoughness							= 0.0f;
		this->ClearCoatNormalTextureWeight					= 0.0f;
		this->ClearCoatNormalWeight							= 0.0f;
		this->ClearCoatNormalTextureIndex					= -1;
		this->ClearCoatNormal								= { 0.0f, 0.0f, 0.0f };
		this->ClearCoatTintTextureWeight					= 0.0f;
		this->ClearCoatTintWeight							= 0.0f;
		this->ClearCoatTintTextureIndex						= -1;
		this->ClearCoatTint									= { 0.0f, 0.0f, 0.0f };
		this->ClearCoatIOR									= 1.0f;
		this->ClearCoatThickness							= 0.0f;
	}

	material::material() {
		this->Name = "";
	}

	material::material(const aiMaterial* Mat, std::string aDirectory, io::file::manager* aFileManager) : material() {
		// Get Material Name
		this->Name = Mat->GetName().C_Str();
		// Get Material Properties.
		// Load Material Property Constants
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
			this->UniformData.Albedo = math::vec<float, 3>(diffuseColor.r, diffuseColor.g, diffuseColor.b);
			this->UniformData.Opacity = opacity;
			// this->UniformData.Transmission = transmission; // What's the difference between this and opacity?
			this->UniformData.AmbientLighting = math::vec<float, 3>(ambientColor.r, ambientColor.g, ambientColor.b);
			this->UniformData.Emissive = math::vec<float, 3>(emissiveColor.r, emissiveColor.g, emissiveColor.b);
			this->UniformData.Specular = math::vec<float, 3>(specularColor.r, specularColor.g, specularColor.b);
			this->UniformData.Shininess = shininess;
			// this->UniformData.AmbientOcclusion = ambientOcclusion;
			this->UniformData.Metallic = metallic;
			this->UniformData.Roughness = roughness;
			this->UniformData.IOR = refractionIndex;
			this->UniformData.SheenColor = math::vec<float, 3>(sheenColor.r, sheenColor.g, sheenColor.b);
			this->UniformData.SheenRoughness = sheenRoughness;
			this->UniformData.ClearCoatStrength = clearCoat;
			this->UniformData.ClearCoatRoughness = clearCoatRoughness;
			// this->UniformData.Anisotropy = anisotropy;
			// this->UniformData.AnisotropyDirection = math::vec<float, 3>(anisotropyDirection.r, anisotropyDirection.g, anisotropyDirection.b); // Removed
		}

		// Get Material Property Textures.
		for (const auto& TextureType : TextureTypeDatabase) {
			std::shared_ptr<gpu::image> LoadedTexture = load_texture(aFileManager, aDirectory, Mat, TextureType.Type);
			if (LoadedTexture != nullptr) {
				// Texture Loaded Successfully
				this->Texture[TextureType.Name] = LoadedTexture;

				// Since texture exists, override the default material constant weights for each loaded texture.
				// ! This is ugly, change later.
				if (TextureType.Name == "Albedo") {
					this->UniformData.AlbedoVertexWeight = 0.0f;
					this->UniformData.AlbedoTextureWeight = 1.0f;
					this->UniformData.AlbedoWeight = 0.0f;
					this->UniformData.AlbedoTextureIndex = 0; // Default to first texture index
				}
				if (TextureType.Name == "Opacity") {
					this->UniformData.OpacityTextureWeight = 1.0f;
					this->UniformData.OpacityWeight = 0.0f;
					this->UniformData.OpacityTextureIndex = 0; // Default to first texture index
				}
				// Not used currently (Transmission?)
				// if (TextureType.Name == "Reflection") {
				// 	this->UniformData.ReflectionTextureWeight = 1.0f;
				// 	this->UniformData.ReflectionWeight = 0.0f;
				// }
				if (TextureType.Name == "Normal") {
					this->UniformData.NormalVertexWeight = 0.0f;
					this->UniformData.NormalTextureWeight = 1.0f;
					this->UniformData.NormalTextureIndex = 0; // Default to first texture index
				}
				if (TextureType.Name == "Height") {
					this->UniformData.HeightTextureIndex = 0; // Default to first texture index
					this->UniformData.HeightScale = 1.0f;
					this->UniformData.HeightStepCount = 1;
				}
				if (TextureType.Name == "AmbientLighting") {
					this->UniformData.AmbientLightingTextureWeight = 1.0f;
					this->UniformData.AmbientLightingWeight = 0.0f;
					this->UniformData.AmbientLightingTextureIndex = 0; // Default to first texture index
				}
				if (TextureType.Name == "Emissive") {
					this->UniformData.EmissiveTextureWeight = 1.0f;
					this->UniformData.EmissiveWeight = 0.0f;
					this->UniformData.EmissiveTextureIndex = 0; // Default to first texture index
				}
				// Find a way to merge these textures into one texture.
				if (TextureType.Name == "Specular") {
					this->UniformData.SpecularTextureWeight = 1.0f;
					this->UniformData.SpecularWeight = 0.0f;
					this->UniformData.SpecularTextureIndex = 0; // Default to first texture index
				}
				if (TextureType.Name == "Shininess") {
					this->UniformData.ShininessTextureWeight = 1.0f;
					this->UniformData.ShininessWeight = 0.0f;
					this->UniformData.ShininessTextureIndex = 0; // Default to first texture index
				}
				if (TextureType.Name == "AmbientOcclusion") {
					this->UniformData.AmbientOcclusionTextureWeight = 1.0f;
					this->UniformData.AmbientOcclusionWeight = 0.0f;
					this->UniformData.AmbientOcclusionTextureIndex = 0; // Default to first texture index
				}
				if (TextureType.Name == "Metallic") {
					this->UniformData.MetallicTextureWeight = 1.0f;
					this->UniformData.MetallicWeight = 0.0f;
					this->UniformData.MetallicTextureIndex = 0;
				}
				if (TextureType.Name == "Roughness") {
					this->UniformData.RoughnessTextureWeight = 1.0f;
					this->UniformData.RoughnessWeight = 0.0f;
					this->UniformData.RoughnessTextureIndex = 0;
				}
				if (TextureType.Name == "Sheen") {
					this->UniformData.SheenIntensityTextureWeight = 1.0f;
					this->UniformData.SheenIntensityWeight = 0.0f;
					this->UniformData.SheenRoughnessTextureWeight = 1.0f;
					this->UniformData.SheenRoughnessWeight = 0.0f;
					this->UniformData.SheenMaskTextureIndex = 0; // Default to first texture index
				}
				if (TextureType.Name == "ClearCoat") {
					this->UniformData.ClearCoatStrengthTextureWeight = 1.0f;
					this->UniformData.ClearCoatStrengthWeight = 0.0f;
					this->UniformData.ClearCoatRoughnessTextureWeight = 1.0f;
					this->UniformData.ClearCoatRoughnessWeight = 0.0f;
					this->UniformData.ClearCoatStrengthRoughnessTextureIndex = 0; // Default to first texture index
				}
			}
			else {
				// Texture does not exist, load default texture
				this->Texture[TextureType.Name] = TextureType.DefaultTexture;
			}
		}

		// TODO: Unpack any packed textures, respect loaded files.
	}

	material::material(std::shared_ptr<gpu::context> aContext, gpu::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial) : material() {
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
			this->Texture[Texture.first] = std::make_shared<gpu::image>(aContext, aCreateInfo, Texture.second);
		}
	}

	material::~material() {}

	void material::update(double aDeltaTime) {
		// Update Material Properties
		// material_data MaterialData = material_data(this);
		// memcpy(this->UniformBuffer->Ptr, &MaterialData, sizeof(material_data));
	}

}

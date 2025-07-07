#pragma once
#ifndef GEODESY_CORE_GFX_MATERIAL_H
#define GEODESY_CORE_GFX_MATERIAL_H

#include <string>
#include <vector>

#include "../io/file.h"

#include "../gpu/image.h"
#include "../gpu/shader.h"

struct aiMaterial;

namespace geodesy::core::gfx {


	// A material describes the qualities of a surface, such as 
	// its roughness, its 
	class material {
	public:

		enum rendering_system : int {
			LEGACY,
			PBR,
			UNKNOWN
		};

		enum transparency : int {
			OPAQUE, 
			TRANSPARENT,
			TRANSLUCENT
		};

		// enum orm_packing : int {};

		struct uniform_data {
			alignas(4) int 								Transparency;
			alignas(4) float 							AlbedoVertexWeight;
			alignas(4) float 							AlbedoTextureWeight;
			alignas(4) float 							AlbedoWeight;
			alignas(4) int 								AlbedoTextureIndex;
			alignas(16) math::vec<float, 3> 			Albedo;
			alignas(4) float 							OpacityTextureWeight;
			alignas(4) float 							OpacityWeight;
			alignas(4) int 								OpacityTextureIndex;
			alignas(4) float 							Opacity;
			alignas(4) float 							NormalVertexWeight;
			alignas(4) float 							NormalTextureWeight;
			alignas(4) int 								NormalTextureIndex;
			alignas(4) int 								HeightTextureIndex;
			alignas(4) float 							HeightScale;
			alignas(4) int 								HeightStepCount;
			alignas(4) float 							AmbientLightingTextureWeight;
			alignas(4) float 							AmbientLightingWeight;
			alignas(4) int 								AmbientLightingTextureIndex;
			alignas(16) math::vec<float, 3> 			AmbientLighting;
			alignas(4) float 							EmissiveTextureWeight;
			alignas(4) float 							EmissiveWeight;
			alignas(4) int 								EmissiveTextureIndex;
			alignas(16) math::vec<float, 3> 			Emissive;
			alignas(4) float 							SpecularTextureWeight;
			alignas(4) float 							SpecularWeight;
			alignas(4) int 								SpecularTextureIndex;
			alignas(16) math::vec<float, 3> 			Specular;
			alignas(4) float 							ShininessTextureWeight;
			alignas(4) float 							ShininessWeight;
			alignas(4) int 								ShininessTextureIndex;
			alignas(4) float 							Shininess;
			alignas(4) float 							AmbientOcclusionTextureWeight;
			alignas(4) float 							AmbientOcclusionWeight;
			alignas(4) int 								AmbientOcclusionTextureIndex;
			alignas(4) float 							AmbientOcclusion;
			alignas(4) float 							RoughnessTextureWeight;
			alignas(4) float 							RoughnessWeight;
			alignas(4) int 								RoughnessTextureIndex;
			alignas(4) float 							Roughness;
			alignas(4) float 							MetallicTextureWeight;
			alignas(4) float 							MetallicWeight;
			alignas(4) int 								MetallicTextureIndex;
			alignas(4) float 							Metallic;
			alignas(4) float 							IORVertexWeight;
			alignas(4) float 							IORTextureWeight;
			alignas(4) float 							IORWeight;
			alignas(4) int 								IORTextureIndex;
			alignas(4) float 							IOR;
			alignas(4) float 							SheenColorTextureWeight;
			alignas(4) float 							SheenColorWeight;
			alignas(4) int 								SheenColorTextureIndex;
			alignas(16) math::vec<float, 3> 			SheenColor;
			alignas(4) float 							SheenIntensityTextureWeight;
			alignas(4) float 							SheenIntensityWeight;
			alignas(4) float 							SheenRoughnessTextureWeight;
			alignas(4) float 							SheenRoughnessWeight;
			alignas(4) int 								SheenMaskTextureIndex;
			alignas(4) float 							SheenRoughness;
			alignas(4) float 							SheenIntensity;
			alignas(4) float 							ClearCoatStrengthTextureWeight;
			alignas(4) float 							ClearCoatStrengthWeight;
			alignas(4) float 							ClearCoatRoughnessTextureWeight;
			alignas(4) float 							ClearCoatRoughnessWeight;
			alignas(4) int 								ClearCoatStrengthRoughnessTextureIndex;
			alignas(4) float 							ClearCoatStrength;
			alignas(4) float 							ClearCoatRoughness;
			alignas(4) float 							ClearCoatNormalTextureWeight;
			alignas(4) float 							ClearCoatNormalWeight;
			alignas(4) int 								ClearCoatNormalTextureIndex;
			alignas(16) math::vec<float, 3> 			ClearCoatNormal;
			alignas(4) float 							ClearCoatTintTextureWeight;
			alignas(4) float 							ClearCoatTintWeight;
			alignas(4) int 								ClearCoatTintTextureIndex;
			alignas(16) math::vec<float, 3> 			ClearCoatTint;
			alignas(4) float 							ClearCoatIOR;
			alignas(4) float 							ClearCoatThickness;
			// TODO: Anistropic Properties
			// TODO: Subsurface Scattering
			// float SubsurfaceScattering;
			uniform_data();
		};

		std::string 											Name;					// Name of the material
		uniform_data 											UniformData;
		std::shared_ptr<gpu::buffer> 							UniformBuffer;			// Uniform Buffer for the Material
		std::map<std::string, std::shared_ptr<gpu::image>> 		Texture;				// Texture Maps of the Material

		material();
		material(const aiMaterial* aMaterial, std::string aDirectory, io::file::manager* aFileManager);
		material(std::shared_ptr<gpu::context> aContext, gpu::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial);
		~material();

		void update(double aDeltaTime);

	};

}

#endif // !GEODESY_CORE_GFX_MATERIAL_H

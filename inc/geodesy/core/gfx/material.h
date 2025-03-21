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
			alignas(4) int 						RenderingSystem; 					// Rendering System for the Material
			alignas(4) int 						Transparency; 						// Transparency of the Material

			// ----- Material Property Constants & Control Weights ----- //
			alignas(4) float 					VertexColorWeight; 					// Determines whether vertex, texture, or constant color is used.
			alignas(4) float 					TextureColorWeight;
			alignas(4) float 					ColorWeight;
			alignas(16) math::vec<float, 3> 	Color;								// Material Color Constant
			
			alignas(4) float 					TextureSpecularWeight; 				// Determines whether texture or constant specular is used.
			alignas(4) float 					SpecularWeight;
			alignas(16) math::vec<float, 3> 	Specular;							// Specular Color of the Material

			alignas(4) float 					TextureAmbientWeight; 				// Determines whether texture or constant ambient is used.
			alignas(4) float 					AmbientWeight;
			alignas(16) math::vec<float, 3> 	AmbientLighting;					// Ambient Lighting Color of the Material

			alignas(4) float 					TextureEmissiveWeight; 				// Determines whether texture or constant emissive is used.
			alignas(4) float 					EmissiveWeight;
			alignas(16) math::vec<float, 3> 	Emissive;							// Emissive Color of the Material

			alignas(4) float 					TextureShininessWeight; 			// Determines whether texture or constant shininess is used.
			alignas(4) float 					ShininessWeight;
			alignas(4) float 					Shininess;							// Shininess of the Material

			alignas(4) float 					TextureOpacityWeight; 				// Determines whether texture or constant opacity is used.
			alignas(4) float 					OpacityWeight;
			alignas(4) float  					Opacity; 							// Opacity of the Material

			alignas(4) float 					VertexNormalWeight; 				// Determines whether vertex normal or texture normal is used.
			alignas(4) float 					TextureNormalWeight;

			alignas(4) float 					TextureAmbientOcclusionWeight; 		// Determines whether texture or constant ambient occlusion is used.
			alignas(4) float 					AmbientOcclusionWeight;
			alignas(4) float 					AmbientOcclusion;

			alignas(4) float 					TextureReflectionWeight; 			// Determines whether texture or constant reflection is used.
			alignas(4) float 					ReflectionWeight;
			alignas(4) float 					Reflection;

			alignas(4) float 					TextureMetallicWeight; 				// Determines whether texture or constant metallic is used.
			alignas(4) float 					MetallicWeight;
			alignas(4) float 					Metallic;							// Metallic Value of the Material

			alignas(4) float 					TextureRoughnessWeight; 			// Determines whether texture or constant roughness is used.
			alignas(4) float 					RoughnessWeight;
			alignas(4) float 					Roughness;							// Roughness Value of the Material

			alignas(4) float 					TextureSheenWeight; 				// Determines whether texture or constant sheen is used.
			alignas(4) float 					SheenWeight;
			alignas(16) math::vec<float, 3> 	SheenColor;							// Sheen Value of the Material
			alignas(4) float 					SheenRoughness;

			alignas(4) float 					TextureClearCoatWeight; 			// Determines whether texture or constant clear coat is used.
			alignas(4) float 					ClearCoatWeight;
			alignas(4) float 					ClearCoat;							// ClearCoat Value of the Material
			alignas(4) float 					ClearCoatRoughness;
			// alignas(4) float ClearCoatNormalStrength; 								// Clear coat normal strength

			// ----- Extraneous Material Properties ----- //
			alignas(4) float 					RefractionIndex;      				// Light bending (Glass/Water)

			alignas(4) float 					Anisotropy;           				// Brushed Metal Reflection
			alignas(16) math::vec<float, 3> 	AnisotropyDirection; 				// Preferred anisotropic direction

			alignas(4) float 					SubsurfaceScattering; 				// Light diffusion under surface

			alignas(4) float 					ParallaxScale;
			alignas(4) int 						ParallaxIterationCount;

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

#pragma once
#ifndef GEODESY_CORE_GFX_MATERIAL_H
#define GEODESY_CORE_GFX_MATERIAL_H

#include <string>
#include <vector>

#include "../io/file.h"

#include "../gcl/image.h"
#include "../gcl/shader.h"

struct aiMaterial;

namespace geodesy::core::gfx {


	// A material describes the qualities of a surface, such as 
	// its roughness, its 
	class material {
	public:

		enum rendering_system {
			LEGACY,
			PBR,
			UNKNOWN
		};

		enum transparency {
			OPAQUE, 
			TRANSPARENT,
			TRANSLUCENT
		};

		enum orm_packing : int {
		};

		struct uniform_data {
			alignas(4) int 						RenderingSystem; 		// Rendering System for the Material
			alignas(4) int 						Transparency; 			// Transparency of the Material


			alignas(16) math::vec<float, 3> 	Color;					// Base Color of the Material
			alignas(16) math::vec<float, 3> 	Emissive;				// Emissive Color of the Material
			alignas(16) math::vec<float, 3> 	Ambient;				// Ambient Color of the Material
			alignas(16) math::vec<float, 3> 	Specular;				// Specular Color of the Material
			alignas(4) float 					Opacity;
			alignas(4) float 					RefractionIndex;
			alignas(4) float 					Shininess;
			alignas(4) float 					Metallic;
			alignas(4) float 					Roughness;
			alignas(4) float 					VertexColorWeight;
			alignas(4) float 					MaterialColorWeight;
			alignas(4) float 					ParallaxScale;
			alignas(4) int 						ParallaxIterationCount;
			uniform_data();
			uniform_data(const material* aMaterial);
		};

		std::string 											Name;					// Name of the material
		rendering_system 										RenderingSystem;		// Rendering System of the Material
		transparency 											Transparency;
		uniform_data 											UniformData;
		std::shared_ptr<gcl::buffer> 							UniformBuffer;			// Uniform Buffer for the Material
		std::map<std::string, std::shared_ptr<gcl::image>> 		Texture;				// Texture Maps of the Material

		material();
		material(const aiMaterial* aMaterial, std::string aDirectory, io::file::manager* aFileManager);
		material(std::shared_ptr<gcl::context> aContext, gcl::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial);
		~material();

		void update(double aDeltaTime);

	};

}

#endif // !GEODESY_CORE_GFX_MATERIAL_H

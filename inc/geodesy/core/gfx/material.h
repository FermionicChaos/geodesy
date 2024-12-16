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

		std::string 											Name;					// Name of the material
		rendering_system 										RenderingSystem;		// Rendering System of the Material
		transparency 											Transparency;

		math::vec<float, 3> 									Color;					// Base Color of the Material
		math::vec<float, 3> 									Emissive;				// Emissive Color of the Material
		math::vec<float, 3> 									Ambient;				// Ambient Color of the Material
		math::vec<float, 3> 									Specular;				// Specular Color of the Material
		float 													Opacity;
		float 													RefractionIndex;
		float 													Shininess;
		float 													Metallic;
		float 													Roughness;
		float 													VertexColorWeight;
		float 													MaterialColorWeight;
		float 													ParallaxScale;
		int 													ParallaxIterationCount;

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

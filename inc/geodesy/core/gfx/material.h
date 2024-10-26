#pragma once
#ifndef GEODESY_CORE_GFX_MATERIAL_H
#define GEODESY_CORE_GFX_MATERIAL_H

#include <string>
#include <vector>

#include "../gcl/image.h"
#include "../gcl/shader.h"

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
		std::map<std::string, std::shared_ptr<gcl::image>> 		Texture;	// Texture Maps of the Material

		material();
		material(std::shared_ptr<gcl::context> aContext, gcl::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial);
		~material();

	};

}

#endif // !GEODESY_CORE_GFX_MATERIAL_H

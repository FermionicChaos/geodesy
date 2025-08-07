#pragma once
#ifndef GEODESY_CORE_GFX_MODEL_H
#define GEODESY_CORE_GFX_MODEL_H

#include <memory>

#include "../../config.h"

#include "../io/file.h"

#include "../phys.h"

#include "mesh.h"
#include "material.h"
#include "node.h"

namespace geodesy::core::gfx {

	class model : public io::file {
	public:

		struct light {

			enum type : int {
				UNDEFINED,
				AMBIENT,
				DIRECTIONAL,
				POINT,
				SPOT, 
				AREA
			};

			alignas(4) int 						Type;
			alignas(4) float 					Intensity;
			alignas(16) math::vec<float, 3> 	Color;
			alignas(16) math::vec<float, 3> 	Position;
			alignas(16) math::vec<float, 3> 	Direction;
			alignas(4) float 					SpotAngle;

			light();
		};

		static bool initialize();
		static void terminate();

		// --------------- Aggregate Model Resources --------------- //

		// Model Metadata
		std::string										Name;
		double 											Time;

		// Resources
		std::shared_ptr<gpu::context> 					Context;
		std::shared_ptr<gfx::node>						Hierarchy;			// Root Node Hierarchy 
		std::vector<phys::animation> 					Animation; 			// Overrides Bind Pose Transform
		std::vector<std::shared_ptr<mesh>> 				Mesh;
		std::vector<std::shared_ptr<material>> 			Material;
		std::vector<std::shared_ptr<gpu::image>> 		Texture;
		std::vector<light> 								Light;				// Not Relevant To Model, open as stage.
		// std::vector<std::shared_ptr<camera>> 		Camera;			// Not Relevant To Model, open as stage.
		// std::shared_ptr<gpu::buffer> 					UniformBuffer;

		model();
		model(std::string aFilePath, file::manager* aFileManager = nullptr);
		model(std::shared_ptr<gpu::context> aContext, std::shared_ptr<model> aModel, gpu::image::create_info aCreateInfo = {});
		~model();

	};

}

#endif // !GEODESY_CORE_GFX_MODEL_H

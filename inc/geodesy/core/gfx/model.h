#pragma once
#ifndef GEODESY_CORE_GFX_MODEL_H
#define GEODESY_CORE_GFX_MODEL_H

#include <memory>

#include "../../config.h"

#include "../io/file.h"

#include "mesh.h"
#include "material.h"
#include "animation.h"
#include "node.h"

namespace geodesy::core::gfx {

	class model : public io::file {
	public:

		static bool initialize();
		static void terminate();

		// --------------- Aggregate Model Resources --------------- //

		// Model Metadata
		std::string										Name;
		double 											Time;
		float 											BindPoseWeight;		// Default Weight of the Node is 1.0f. 

		// Resources
		std::shared_ptr<gcl::context> 					Context;
		node											Hierarchy;			// Root Node Hierarchy 
		std::vector<animation> 							Animation; 			// Overrides Bind Pose Transform
		std::vector<std::shared_ptr<mesh>> 				Mesh;
		std::vector<std::shared_ptr<material>> 			Material;
		std::vector<std::shared_ptr<gcl::image>> 		Texture;
		// std::vector<std::shared_ptr<light>> 			Light;				// Not Relevant To Model, open as stage.
		// std::vector<std::shared_ptr<camera>> 		Camera;			// Not Relevant To Model, open as stage.
		// std::shared_ptr<gcl::buffer> 					UniformBuffer;

		model();
		model(std::string aFilePath, file::manager* aFileManager = nullptr);
		model(std::shared_ptr<gcl::context> aContext, std::shared_ptr<model> aModel, gcl::image::create_info aCreateInfo = {});
		~model();

		void update(double aDeltaTime);

	};

}

#endif // !GEODESY_CORE_GFX_MODEL_H

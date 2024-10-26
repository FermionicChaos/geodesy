#pragma once
#ifndef GEODESY_CORE_GFX_RENDERER_H
#define GEODESY_CORE_GFX_RENDERER_H

#include "../../config.h"
#include "../gcl.h"
#include "model.h"

// render_operation is designed to be a component
// of object_t which is extensible in describing
// how an object_t wishes to draw itself.
//

namespace geodesy::core::gfx {	

	// A draw call represents a singular draw call for a single mesh instance in
	// the node hiearchy of the model. Distance from the camera is determined
	struct draw_call {
		VkCommandBuffer 				Command;
		float 							DistanceFromSubject;
		material::transparency 			TransparencyMode;
		draw_call();
	};	

	class renderer {
	public:

		gcl::context*											Context;
		ecs::object* 											Object;
		ecs::subject* 											Subject;
		VkDescriptorPool 										DescriptorPool;
		std::vector<std::shared_ptr<gcl::framebuffer>> 			Framebuffer;
		std::vector<std::shared_ptr<gcl::descriptor::array>> 	DescriptorArray;
		std::vector<std::vector<draw_call>> 					DrawCall;			// [FrameIndex][MeshIndex]

		renderer(core::gcl::context* aContext, ecs::subject* aSubject, ecs::object* aObject);
		~renderer();

	};

}

#endif // !GEODESY_CORE_GFX_RENDERER_H

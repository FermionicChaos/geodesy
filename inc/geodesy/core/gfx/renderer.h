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
		float 											DistanceFromSubject;
		material::transparency 							TransparencyMode;
		std::shared_ptr<gcl::context> 					Context;
		std::shared_ptr<gcl::framebuffer> 				Framebuffer;
		std::shared_ptr<gcl::descriptor::array> 		DescriptorArray;
		VkCommandBuffer 								DrawCommand;
		draw_call();
	};

	class renderer : public std::vector<std::vector<draw_call>> {
	public:

		gcl::context*											Context;
		ecs::object* 											Object;
		ecs::subject* 											Subject;

		renderer(core::gcl::context* aContext, ecs::subject* aSubject, ecs::object* aObject);
		~renderer();

	};

	std::vector<VkCommandBuffer> convert(std::vector<draw_call> aDrawCallList);

}

#endif // !GEODESY_CORE_GFX_RENDERER_H

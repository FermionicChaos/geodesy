#include <geodesy/core/gfx/renderer.h>

// Base objects which the game engine processes.
#include <geodesy/ecs/object.h>
#include <geodesy/ecs/subject.h>
#include <geodesy/ecs/stage.h>
#include <geodesy/ecs/app.h>

namespace geodesy::core::gfx {

	draw_call::draw_call() {
		DistanceFromSubject 	= 0.0f;
		TransparencyMode 		= material::transparency::OPAQUE;
		DrawCommand 				= VK_NULL_HANDLE;
	}

	renderer::renderer(core::gcl::context* aContext, ecs::subject* aSubject, ecs::object* aObject) {
		// The way descriptor sets are going to be allocated is b
		VkResult Result = VK_SUCCESS;

	}
		

	renderer::~renderer() {

	}

	std::vector<VkCommandBuffer> convert(std::vector<draw_call> aDrawCallList) {
		std::vector<VkCommandBuffer> CommandBufferList(aDrawCallList.size());
		for (size_t i = 0; i < aDrawCallList.size(); i++) {
			CommandBufferList[i] = aDrawCallList[i].DrawCommand;
		}
		return CommandBufferList;
	}

}

#include <geodesy/core/gfx/renderer.h>

// Base objects which the game engine processes.
#include <geodesy/ecs/object.h>
#include <geodesy/ecs/subject.h>
#include <geodesy/ecs/stage.h>
#include <geodesy/ecs/app.h>

namespace geodesy::core::gfx {

	draw_call::draw_call() {
		Command 				= VK_NULL_HANDLE;
		DistanceFromSubject 	= 0.0f;
		TransparencyMode 		= material::transparency::OPAQUE;
	}

	renderer::renderer(core::gcl::context* aContext, ecs::subject* aSubject, ecs::object* aObject) {
		// The way descriptor sets are going to be allocated is b
		VkResult Result = VK_SUCCESS;

	}
		

	renderer::~renderer() {

	}

}

#include <geodesy/bltn/obj/triangle.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gcl;
	using namespace gfx;

	triangle::triangle(
		std::shared_ptr<core::gcl::context> aContext, 
		ecs::stage* aStage, 
		std::string aName
	) : ecs::object(
		aContext, 
		aStage, 
		aName, 
		"../glTF-Sample-Models/2.0/Lantern/glTF/Lantern.gltf",
		{ 0.0f, 10.0f, -10.0f }, 
		{ -90.0f, 0.0f }
	) {
		
	}

}

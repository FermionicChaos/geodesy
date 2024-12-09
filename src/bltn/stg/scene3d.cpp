#include <geodesy/bltn/stg/scene3d.h>

namespace geodesy::bltn::stg {

	using namespace core;

	scene3d::scene3d(std::shared_ptr<core::gcl::context> aContext, std::string aName) : ecs::stage(aContext, aName) {
		math::vec<uint, 3> Resolution = { 640, 480, 1 };
		double FrameRate = 60.0f;
		uint32_t FrameCount = 4;
		this->create_object<obj::camera3d>("Camera3D", Resolution, FrameRate, FrameCount);
		this->create_object<obj::triangle>("Triangle");
	}

}

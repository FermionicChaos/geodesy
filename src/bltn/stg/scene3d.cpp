#include <geodesy/bltn/stg/scene3d.h>

namespace geodesy::bltn::stg {

	scene3d::scene3d(std::shared_ptr<core::gcl::context> aContext, std::string aName) : ecs::stage(aContext, aName) {
		this->Object.push_back(std::make_shared<obj::camera3d>(aContext, (ecs::stage*)this, std::string("Camera 3D"), core::math::vec<uint, 3>(1920, 1080, 1), 60.0, 3));
		this->Object.push_back(std::make_shared<obj::triangle>(aContext, (ecs::stage*)this, std::string("Triangle")));
	}

}

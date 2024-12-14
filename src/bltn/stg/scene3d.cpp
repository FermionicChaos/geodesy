#include <geodesy/bltn/stg/scene3d.h>

namespace geodesy::bltn::stg {

	using namespace core;

	scene3d::scene3d(std::shared_ptr<core::gcl::context> aContext, std::string aName) : ecs::stage(aContext, aName) {
		obj::camera3d::create_info CameraCreateInfo;
		CameraCreateInfo.Resolution = { 1280, 720, 1 };
		CameraCreateInfo.FrameRate = 60.0f;
		CameraCreateInfo.FrameCount = 4;
		CameraCreateInfo.FOV = 70.0f;
		CameraCreateInfo.Near = 0.1f;
		CameraCreateInfo.Far = 1000.0f;
		math::vec<uint, 3> Resolution = { 1280, 720, 1 };
		double FrameRate = 60.0f;
		uint32_t FrameCount = 4;
		this->create_object<obj::camera3d>("Camera3D", CameraCreateInfo, math::vec<float, 3>(0.0f, -10.0f, 10.0f));

		std::vector<std::string> SpawnObjects = {
			"../glTF-Sample-Models/2.0/Box/glTF/Box.gltf",
			"../glTF-Sample-Models/2.0/Lantern/glTF/Lantern.gltf",
		};

		for (size_t i = 0; i < SpawnObjects.size(); i++) {
			std::string ObjectName = "Object" + std::to_string(i);
			math::vec<float, 3> Position = { 5.0f * (float)i, 0.0f, 0.0f };
			math::vec<float, 2> Direction = { -90.0f, 0.0f };
			this->create_object<ecs::object>(ObjectName, SpawnObjects[i], Position, Direction);
		}
	}

}

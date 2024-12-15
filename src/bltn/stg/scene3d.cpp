#include <geodesy/bltn/stg/scene3d.h>

namespace geodesy::bltn::stg {

	using namespace core;

	scene3d::scene3d(std::shared_ptr<core::gcl::context> aContext, std::string aName) : ecs::stage(aContext, aName) {
		obj::camera3d::create_info CameraCreateInfo;
		CameraCreateInfo.Resolution 	= { 1280, 720, 1 };
		CameraCreateInfo.FrameRate 		= 60.0f;
		CameraCreateInfo.FrameCount 	= 4;
		CameraCreateInfo.FOV 			= 70.0f;
		CameraCreateInfo.Near 			= 0.1f;
		CameraCreateInfo.Far 			= 1000.0f;
		this->create_object<obj::camera3d>("Camera3D", CameraCreateInfo, math::vec<float, 3>(0.0f, -50.0f, 10.0f));

		std::vector<std::string> SpawnObjects = {
			"../glTF-Sample-Models/2.0/Box/glTF/Box.gltf",
			"../glTF-Sample-Models/2.0/BoxTextured/glTF/BoxTextured.gltf",
			"../glTF-Sample-Models/2.0/Lantern/glTF/Lantern.gltf",
			"../glTF-Sample-Models/2.0/CesiumMilkTruck/glTF/CesiumMilkTruck.gltf",
			"assets/models/Pigwithanimation.gltf",
			"../glTF-Sample-Models/2.0/BrainStem/glTF/BrainStem.gltf",
			"../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf",
		};

		// for (size_t i = 0; i < SpawnObjects.size(); i++) {
		// 	std::string ObjectName = "Object" + std::to_string(i);
		// 	math::vec<float, 3> Position = { 10.0f * (float)i, 0.0f, 0.0f };
		// 	math::vec<float, 2> Direction = { -90.0f, 0.0f };
		// 	this->create_object<ecs::object>(ObjectName, SpawnObjects[i], Position, Direction);
		// }

		this->create_object<ecs::object>("Box", 				SpawnObjects[0], 	math::vec<float, 3>(0.0f, 0.0f, 0.0f), 		math::vec<float, 2>(0.0f, 0.0f), 	math::vec<float, 3>(5.0f, 5.0f, 5.0f));
		this->create_object<ecs::object>("BoxTextured", 		SpawnObjects[1], 	math::vec<float, 3>(10.0f, 0.0f, 0.0f), 	math::vec<float, 2>(0.0f, 0.0f), 	math::vec<float, 3>(5.0f, 5.0f, 5.0f));
		this->create_object<ecs::object>("Lantern", 			SpawnObjects[2], 	math::vec<float, 3>(20.0f, 0.0f, 0.0f), 	math::vec<float, 2>(-90.0f, 0.0f), 	math::vec<float, 3>(1.0f, 1.0f, 1.0f));
		this->create_object<ecs::object>("CesiumMilkTruck", 	SpawnObjects[3], 	math::vec<float, 3>(0.0f, 30.0f, 0.0f), 	math::vec<float, 2>(180.0f, 0.0f), 	math::vec<float, 3>(5.0f, 5.0f, 5.0f));
		// this->create_object<ecs::object>("Pigwithanimation", 	SpawnObjects[4], 	math::vec<float, 3>(-40.0f, 30.0f, 0.0f), 	math::vec<float, 2>(-90.0f, 0.0f), 	math::vec<float, 3>(5.0f, 5.0f, 5.0f));
		this->create_object<ecs::object>("BrainStem", 			SpawnObjects[5], 	math::vec<float, 3>(40.0f, 30.0f, 0.0f), 	math::vec<float, 2>(0.0f, 0.0f), 	math::vec<float, 3>(10.0f, 10.0f, 10.0f));
		this->create_object<ecs::object>("Sponza", 				SpawnObjects[6], 	math::vec<float, 3>(0.0f, 0.0f, 0.0f), 		math::vec<float, 2>(-90.0f, 90.0f), 	math::vec<float, 3>(75.0f, 75.0f, 75.0f));
	}

}

#include <geodesy/bltn/stg/scene3d.h>

namespace geodesy::bltn::stg {

	using namespace core;

	scene3d::scene3d(std::shared_ptr<core::gpu::context> aContext, std::string aName) : runtime::stage(aContext, aName) {
		// Create Info for stage camera.
		obj::camera3d::creator CameraCreateInfo;
		CameraCreateInfo.Name 				= "Camera3D";
		CameraCreateInfo.Position 			= { 0.0f, -50.0f, 10.0f };
		CameraCreateInfo.Direction 			= { 0.0f, 0.0f };
		CameraCreateInfo.Resolution 		= { 1920, 1080, 1 };
		CameraCreateInfo.FrameRate 			= 60.0f;
		CameraCreateInfo.FrameCount 		= 4;
		CameraCreateInfo.FOV 				= 70.0f;
		CameraCreateInfo.Near 				= 0.1f;
		CameraCreateInfo.Far 				= 2000.0f;

		// Create Temp Array of objects to be created..
		std::vector<runtime::object::creator> ObjectList;

		// runtime::object::creator Box;
		// Box.Name 							= "Box";
		// Box.ModelPath 						= "dep/gltf-models/2.0/Box/glTF/Box.gltf";
		// Box.Position 						= { 0.0f, 0.0f, 0.0f };
		// Box.Direction 						= { 0.0f, 0.0f };
		// Box.Scale 							= { 5.0f, 5.0f, 5.0f };
		// ObjectList.push_back(Box);

		// runtime::object::creator BoxTextured;
		// BoxTextured.Name 					= "BoxTextured";
		// BoxTextured.ModelPath 				= "dep/gltf-models/2.0/BoxTextured/glTF/BoxTextured.gltf";
		// BoxTextured.Position 				= { 10.0f, 0.0f, 0.0f };
		// BoxTextured.Direction 				= { 0.0f, 0.0f };
		// BoxTextured.Scale 					= { 5.0f, 5.0f, 5.0f };
		// ObjectList.push_back(BoxTextured);

		runtime::object::creator Lantern;
		Lantern.Name 						= "Lantern";
		Lantern.ModelPath 					= "dep/gltf-models/2.0/Lantern/glTF/Lantern.gltf";
		Lantern.Position 					= { 00.0f, 0.0f, 0.0f };
		Lantern.Direction 					= { -90.0f, 0.0f };
		Lantern.Scale 						= { 1.0f, 1.0f, 1.0f };
		ObjectList.push_back(Lantern);

		runtime::object::creator Sponza;
		Sponza.Name 						= "Sponza";
		Sponza.ModelPath 					= "dep/gltf-models/2.0/Sponza/glTF/Sponza.gltf";
		Sponza.Position 					= { 0.0f, 0.0f, 0.0f };
		Sponza.Direction 					= { -90.0f, 90.0f };
		Sponza.Scale 						= { 75.0f, 75.0f, 75.0f };
		ObjectList.push_back(Sponza);

		// runtime::object::creator CesiumMilkTruck;
		// CesiumMilkTruck.Name 				= "CesiumMilkTruck";
		// CesiumMilkTruck.ModelPath 			= "dep/gltf-models/2.0/CesiumMilkTruck/glTF/CesiumMilkTruck.gltf";
		// CesiumMilkTruck.Position 			= { 0.0f, 30.0f, 0.0f };
		// CesiumMilkTruck.Direction 			= { -90.0f, 0.0f };
		// CesiumMilkTruck.Scale 				= { 5.0f, 5.0f, 5.0f };
		// CesiumMilkTruck.AnimationWeights 	= { 0.0f, 1.0f };
		// ObjectList.push_back(CesiumMilkTruck);

		// runtime::object::creator Pigwithanimation;
		// Pigwithanimation.Name 				= "Pigwithanimation";
		// Pigwithanimation.ModelPath 			= "assets/models/Pigwithanimation.gltf";
		// Pigwithanimation.Position 			= { -40.0f, 30.0f, 0.0f };
		// Pigwithanimation.Direction 			= { -90.0f, 0.0f };
		// Pigwithanimation.Scale 				= { 5.0f, 5.0f, 5.0f };
		// Pigwithanimation.AnimationWeights 	= { 1.0f, 0.0f };
		// ObjectList.push_back(Pigwithanimation);

		runtime::object::creator BrainStem;
		BrainStem.Name 						= "BrainStem";
		BrainStem.ModelPath 				= "dep/gltf-models/2.0/BrainStem/glTF/BrainStem.gltf";
		BrainStem.Position 					= { 10.0f, 00.0f, 0.0f };
		BrainStem.Direction 				= { -90.0f, 0.0f };
		BrainStem.Scale 					= { 10.0f, 10.0f, 10.0f };
		BrainStem.AnimationWeights 			= { 0.0f, 1.0f };
		ObjectList.push_back(BrainStem);

		runtime::object::creator CesiumMan;
		CesiumMan.Name 						= "CesiumMan";
		CesiumMan.ModelPath 				= "dep/gltf-models/2.0/CesiumMan/glTF/CesiumMan.gltf";
		CesiumMan.Position 					= { -10.0f, 0.0f, 0.0f };
		CesiumMan.Direction 				= { -90.0f, 0.0f };
		CesiumMan.Scale 					= { 10.0f, 10.0f, 10.0f };
		CesiumMan.AnimationWeights 			= { 0.0f, 1.0f };
		ObjectList.push_back(CesiumMan);

		// runtime::object::creator ParallaxPlane;
		// ParallaxPlane.Name 					= "ParallaxPlane";
		// ParallaxPlane.ModelPath 			= "assets/models/bricks2/bricks2.gltf";
		// ParallaxPlane.Position 				= { 0.0f, 0.0f, 5.0f };
		// ParallaxPlane.Direction 			= { -90.0f, 0.0f };
		// ParallaxPlane.Scale 				= { 5.0f, 5.0f, 5.0f };
		// ObjectList.push_back(ParallaxPlane);

		// Create Camera.
		this->create_object<obj::camera3d>(&CameraCreateInfo);
		// Create objects.
		for (size_t i = 0; i < ObjectList.size(); i++) {
			this->create_object<runtime::object>(&ObjectList[i]);
		}

	}

}

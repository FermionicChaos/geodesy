#include <geodesy/bltn/stg/scene3d.h>

namespace geodesy::bltn::stg {

	using namespace core;

	scene3d::scene3d(std::shared_ptr<core::gpu::context> aContext, std::string aName) : runtime::stage(aContext, aName) {
		// Create Info for stage camera.
		obj::camera3d::creator CameraCreateInfo;
		CameraCreateInfo.Name 				= "Camera3D";
		CameraCreateInfo.Position 			= { 0.0f, -10.0f, 5.0f };
		CameraCreateInfo.Direction 			= { 0.0f, 0.0f };
		CameraCreateInfo.Resolution 		= { 2*1920, 2*1080, 1 };
		CameraCreateInfo.FrameRate 			= 500.0f;
		CameraCreateInfo.FrameCount 		= 1;
		CameraCreateInfo.FOV 				= 70.0f;
		CameraCreateInfo.Near 				= 1.0f;
		CameraCreateInfo.Far 				= 2000.0f;

		// Create Temp Array of objects to be created..
		std::vector<runtime::object::creator> ObjectList;

		runtime::object::creator Sponza;
		Sponza.Name 						= "Sponza";
		Sponza.ModelPath 					= "dep/gltf-models/2.0/Sponza/glTF/Sponza.gltf";
		Sponza.Position 					= { 0.0f, 0.0f, 0.0f };
		Sponza.Direction 					= { -90.0f, 90.0 };
		Sponza.Scale 						= { 0.03f, 0.03f, 0.03f };
		ObjectList.push_back(Sponza);

		runtime::object::creator PirateMap;
		PirateMap.Name 						= "PirateMap";
		PirateMap.ModelPath 				= "assets/models/pirate_map/scene.gltf";
		PirateMap.Position 					= { 0.0f, 0.0f, 0.0f };
		PirateMap.Direction 				= { 0.0f, 0.0f };
		PirateMap.Scale 					= { 0.003f, 0.003f, 0.003f };
		ObjectList.push_back(PirateMap);

		// Simple Boxes

		runtime::object::creator Box;
		Box.Name 							= "Box";
		Box.ModelPath 						= "dep/gltf-models/2.0/Box/glTF/Box.gltf";
		Box.Position 						= { 0.0f, 0.0f, 0.0f };
		Box.Direction 						= { 0.0f, 0.0f };
		Box.Scale 							= { 1.0f, 1.0f, 1.0f };
		ObjectList.push_back(Box);

		runtime::object::creator BoxTextured;
		BoxTextured.Name 					= "BoxTextured";
		BoxTextured.ModelPath 				= "dep/gltf-models/2.0/BoxTextured/glTF/BoxTextured.gltf";
		BoxTextured.Position 				= { 2.0f, 0.0f, 0.0f };
		BoxTextured.Direction 				= { 0.0f, 0.0f };
		BoxTextured.Scale 					= { 1.0f, 1.0f, 1.0f };
		ObjectList.push_back(BoxTextured);

		runtime::object::creator BoxTextured2;
		BoxTextured2.Name 					= "BoxTextured2";
		BoxTextured2.ModelPath 				= "dep/gltf-models/2.0/BoxTextured/glTF/BoxTextured.gltf";
		BoxTextured2.Position 				= { 4.0f, 0.0f, 0.0f };
		BoxTextured2.Direction 				= { 0.0f, 0.0f };
		BoxTextured2.Scale 					= { 1.0f, 1.0f, 1.0f };
		ObjectList.push_back(BoxTextured2);

		runtime::object::creator Cube;
		Cube.Name 							= "Cube";
		Cube.ModelPath 						= "dep/gltf-models/2.0/Cube/glTF/Cube.gltf";
		Cube.Position 						= { 0.0f, 2.0f, 0.0f };
		Cube.Direction 						= { 0.0f, 0.0f };
		Cube.Scale 							= { 0.5f, 0.5f, 0.5f };
		ObjectList.push_back(Cube);

		runtime::object::creator Cube2;
		Cube2.Name 							= "Cube2";
		Cube2.ModelPath 					= "dep/gltf-models/2.0/Cube/glTF/Cube.gltf";
		Cube2.Position 						= { 0.0f, 4.0f, 0.0f };
		Cube2.Direction 					= { 0.0f, 0.0f };
		Cube2.Scale 						= { 0.5f, 0.5f, 0.5f };
		ObjectList.push_back(Cube2);

		runtime::object::creator BoxWithVertexColors;
		BoxWithVertexColors.Name 			= "BoxWithVertexColors";
		BoxWithVertexColors.ModelPath 		= "dep/gltf-models/2.0/BoxTextured/glTF/BoxTextured.gltf";
		BoxWithVertexColors.Position 		= { 0.0f, 0.0f, 2.0f };
		BoxWithVertexColors.Direction 		= { 0.0f, 0.0f };
		BoxWithVertexColors.Scale 			= { 1.0f, 1.0f, 1.0f };
		ObjectList.push_back(BoxWithVertexColors);

		// Gizmo Object

		runtime::object::creator Gizmo;
		Gizmo.Name 							= "Gizmo";
		Gizmo.ModelPath 					= "assets/models/gizmo/scene.gltf";
		Gizmo.Position 						= { 0.0f, 0.0f, 0.0f };
		Gizmo.Direction 					= { 0.0f, 0.0f };
		Gizmo.Scale 						= { 1.0f, 1.0f, 1.0f };
		ObjectList.push_back(Gizmo);

		// Tests Complex Animations

		runtime::object::creator BrainStem;
		BrainStem.Name 						= "BrainStem";
		BrainStem.ModelPath 				= "dep/gltf-models/2.0/BrainStem/glTF/BrainStem.gltf";
		BrainStem.Position 					= { 2.0f, -2.0f, 0.0f };
		BrainStem.Direction 				= { 0.0f, 0.0f };
		BrainStem.Scale 					= { 1.0f, 1.0f, 1.0f };
		BrainStem.AnimationWeights 			= { 0.0f, 1.0f };
		ObjectList.push_back(BrainStem);

		runtime::object::creator CesiumMilkTruck;
		CesiumMilkTruck.Name 				= "CesiumMilkTruck";
		CesiumMilkTruck.ModelPath 			= "dep/gltf-models/2.0/CesiumMilkTruck/glTF/CesiumMilkTruck.gltf";
		CesiumMilkTruck.Position 			= { 0.0f, 12.0f, 0.0f };
		CesiumMilkTruck.Direction 			= { -180.0f, 0.0f };
		CesiumMilkTruck.Scale 				= { 1.0f, 1.0f, 1.0f };
		CesiumMilkTruck.AnimationWeights 	= { 0.0f, 1.0f };
		ObjectList.push_back(CesiumMilkTruck);

		runtime::object::creator CesiumMan;
		CesiumMan.Name 						= "CesiumMan";
		CesiumMan.ModelPath 				= "dep/gltf-models/2.0/CesiumMan/glTF/CesiumMan.gltf";
		CesiumMan.Position 					= { -2.0f, -2.0f, 0.0f };
		CesiumMan.Direction 				= { 0.0f, 0.0f };
		CesiumMan.Scale 					= { 1.0f, 1.0f, 1.0f };
		CesiumMan.AnimationWeights 			= { 0.0f, 1.0f };
		ObjectList.push_back(CesiumMan);

		// Tests Parallax Mapping.
		// runtime::object::creator ParallaxPlane;
		// ParallaxPlane.Name 					= "ParallaxPlane";
		// ParallaxPlane.ModelPath 			= "assets/models/bricks2/bricks2.gltf";
		// ParallaxPlane.Position 				= { 0.0f, 0.0f, 5.0f };
		// ParallaxPlane.Direction 			= { -90.0f, 0.0f };
		// ParallaxPlane.Scale 				= { 5.0f, 5.0f, 5.0f };
		// ObjectList.push_back(ParallaxPlane);

		// Test Emissive Lighting
		runtime::object::creator Lantern;
		Lantern.Name 						= "Lantern";
		Lantern.ModelPath 					= "dep/gltf-models/2.0/Lantern/glTF/Lantern.gltf";
		Lantern.Position 					= { 0.0f, -2.0f, 0.0f };
		Lantern.Direction 					= { -90.0f, 0.0f };
		Lantern.Scale 						= { 0.2f, 0.2f, 0.2f };
		ObjectList.push_back(Lantern);

		// Full PBR Test
		runtime::object::creator DamagedHelmet;
		DamagedHelmet.Name 					= "DamagedHelmet";
		DamagedHelmet.ModelPath 			= "dep/gltf-models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf";
		DamagedHelmet.Position 				= { 8.0f, 0.0f, 0.0f };
		DamagedHelmet.Direction 			= { -180.0f, 0.0f };
		DamagedHelmet.Scale 				= { 1.0f, 1.0f, 1.0f };
		ObjectList.push_back(DamagedHelmet);

		// Special Case Objects

		runtime::object::creator IridescenceLamp;
		IridescenceLamp.Name 				= "IridescenceLamp";
		IridescenceLamp.ModelPath 			= "dep/gltf-models/2.0/IridescenceLamp/glTF/IridescenceLamp.gltf";
		IridescenceLamp.Position 			= { 0.0f, -10.0f, 0.0f };
		IridescenceLamp.Direction 			= { -90.0f, 0.0f };
		IridescenceLamp.Scale 				= { 2.0f, 2.0f, 2.0f };
		ObjectList.push_back(IridescenceLamp);

		runtime::object::creator MosquitoInAmber;
		MosquitoInAmber.Name 				= "MosquitoInAmber";
		MosquitoInAmber.ModelPath 			= "dep/gltf-models/2.0/MosquitoInAmber/glTF/MosquitoInAmber.gltf";
		MosquitoInAmber.Position 			= { -10.0f, 0.0f, 0.0f };
		MosquitoInAmber.Direction 			= { 0.0f, 0.0f };
		MosquitoInAmber.Scale 				= { 10.0f, 10.0f, 10.0f };
		ObjectList.push_back(MosquitoInAmber);

		// // Sheen Rendering.

		// runtime::object::creator SheenCloth;
		// SheenCloth.Name 						= "SheenCloth";
		// SheenCloth.ModelPath 					= "dep/gltf-models/2.0/SheenCloth/glTF/SheenCloth.gltf";
		// SheenCloth.Position 					= { 0.0f, 0.0f, 0.0f };
		// SheenCloth.Direction 					= { -90.0f, 0.0f };
		// SheenCloth.Scale 						= { 1.0f, 1.0f, 1.0f };
		// ObjectList.push_back(SheenCloth);

		// // Test Clear Coat Rendering.

		// runtime::object::creator ToyCar;
		// ToyCar.Name 							= "ToyCar";
		// ToyCar.ModelPath 						= "dep/gltf-models/2.0/ToyCar/glTF/ToyCar.gltf";
		// ToyCar.Position 						= { 0.0f, 0.0f, 0.0f };
		// ToyCar.Direction 						= { -90.0f, 0.0f };
		// ToyCar.Scale 							= { 1.0f, 1.0f, 1.0f };
		// ObjectList.push_back(ToyCar);

		// Create Camera.
		this->create_object<obj::camera3d>(&CameraCreateInfo);
		// Create objects.
		for (size_t i = 0; i < ObjectList.size(); i++) {
			this->create_object<runtime::object>(&ObjectList[i]);
		}

		stage::light_uniform_data* LightBuffer = (stage::light_uniform_data*)this->LightUniformBuffer->Ptr;

		LightBuffer->Source[0] = gfx::model::light(1.0f, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 2.0f});
		LightBuffer->Source[1] = gfx::model::light(3.0f, {1.0f, 0.0f, 0.0f}, {5.0f, 0.0f, 2.0f});
		LightBuffer->Source[2] = gfx::model::light(1.0f, {0.0f, 1.0f, 0.0f}, {0.0f, 5.0f, 2.0f});
		LightBuffer->Source[3] = gfx::model::light(2.0f, {0.0f, 0.0f, 1.0f}, {5.0f, 5.0f, 2.0f});
		LightBuffer->Source[4] = gfx::model::light(6.0f, {1.0f, 1.0f, 0.0f}, {-5.0f, 0.0f, 2.0f});
		LightBuffer->Source[5] = gfx::model::light(4.0f, {1.0f, 0.0f, 1.0f}, {-5.0f, -5.0f, 2.0f});
		LightBuffer->Source[6] = gfx::model::light(10.0f, {0.0f, 1.0f, 1.0f}, {-5.0f, -10.0f, 2.0f});
		LightBuffer->Source[7] = gfx::model::light(4.0f, {1.0f, 1.0f, 1.0f}, {-10.0f, -10.0f, 2.0f});
		LightBuffer->Count = 8;

		this->build_scene_geometry();

	}

}

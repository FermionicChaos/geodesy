#include <geodesy/bltn/obj/camera3d.h>

#include <geodesy/runtime/stage.h>

#include <iostream>
#include <algorithm>

namespace geodesy::bltn::obj {

	using namespace geodesy::core;
	using namespace geodesy::core::gfx;
	using namespace geodesy::core::gpu;

	camera3d::geometry_buffer::geometry_buffer(
		std::shared_ptr<context> 	aContext, 
		math::vec<uint, 3> 			aResolution, 
		double 						aFrameRate, 
		size_t 						aFrameCount
	) : framechain(
		aContext, 
		aFrameRate, 
		aFrameCount
	) {
		// New API design?
		image::create_info DepthCreateInfo;
		DepthCreateInfo.Layout		= image::layout::SHADER_READ_ONLY_OPTIMAL;
		DepthCreateInfo.Memory		= device::memory::DEVICE_LOCAL;
		DepthCreateInfo.Usage		= image::usage::SAMPLED | image::usage::DEPTH_STENCIL_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		image::create_info ColorCreateInfo;
		ColorCreateInfo.Layout		= image::layout::SHADER_READ_ONLY_OPTIMAL;
		ColorCreateInfo.Memory		= device::memory::DEVICE_LOCAL;
		ColorCreateInfo.Usage		= image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::STORAGE | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		// ? Note: Why does half precision PositionFormat cause graphical artifacts unlike full precision floats?
		// Ultra-mobile-compatible formats (supports even oldest mobile GPUs)
		image::format AlbedoFormat = image::format::R8G8B8A8_UNORM; 			// Albedo colors (normalized 0-1)
		image::format PositionFormat = image::format::R16G16B16A16_SFLOAT; 		// World positions (half precision for mobile)
		image::format NormalFormat = image::format::R8G8B8A8_UNORM; 			// Normals (normalized 0-1, can be packed)
		image::format MaterialFormat = image::format::R8G8B8A8_UNORM; 			// Material properties (0-1 range)
		image::format EmissiveFormat = image::format::R8G8B8A8_UNORM; 			// LDR emissive (maximum compatibility)
		image::format DepthFormat = image::format::D16_UNORM; 					// Most compatible depth format
		image::format ColorFormat = image::format::R16G16B16A16_SFLOAT; 		// HDR final output (maximum compatibility)

		this->Resolution = aResolution;
		for (std::size_t i = 0; i < this->Image.size(); i++) {
			// This is the finalized color output.
			// Modified Geometry Buffer for Hybrid Rendering.
			this->Image[i]["OGB.Color"] 			= aContext->create_image(ColorCreateInfo, AlbedoFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Position"] 			= aContext->create_image(ColorCreateInfo, PositionFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Normal"] 			= aContext->create_image(ColorCreateInfo, NormalFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Tangent"] 			= aContext->create_image(ColorCreateInfo, NormalFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Bitangent"] 		= aContext->create_image(ColorCreateInfo, NormalFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.ORM"] 				= aContext->create_image(ColorCreateInfo, MaterialFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Emissive"] 			= aContext->create_image(ColorCreateInfo, EmissiveFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.TranslucencyMask"] 	= aContext->create_image(ColorCreateInfo, MaterialFormat, aResolution[0], aResolution[1]);
			// Shared Depth Buffer for all images.
			this->Image[i]["Depth"] 				= aContext->create_image(DepthCreateInfo, DepthFormat, aResolution[0], aResolution[1]);
			// Final Output after post processing.
			this->Image[i]["Color"] 				= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
		}

		// Setup frame clearing commands.
		this->PredrawFrameOperation = std::vector<command_batch>(this->Image.size());
		this->PostdrawFrameOperation = std::vector<command_batch>(this->Image.size());
		for (size_t i = 0; i < this->Image.size(); i++) {
			// Creates Clearing command for images in frame chain.
			VkCommandBuffer	ClearCommand = aContext->allocate_command_buffer(device::operation::GRAPHICS_AND_COMPUTE);
			aContext->begin(ClearCommand);
			// Opaque Geometry Buffer, used for Opaque.
			this->Image[i]["OGB.Color"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Position"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Normal"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Tangent"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Bitangent"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.ORM"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Emissive"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.TranslucencyMask"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			// Shared Depth Buffer for all images.
			this->Image[i]["Depth"]->clear_depth(ClearCommand, { 0.0f, 0 });
			// Final Output Color Image.
			this->Image[i]["Color"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			aContext->end(ClearCommand);
			this->PredrawFrameOperation[i] += ClearCommand;
		}

	}

	camera3d::rasterizer_draw_call::rasterizer_draw_call(
		camera3d* 	aCamera3D,
		size_t 		aFrameIndex,
		object* 	aObject, 
		size_t 		aMeshInstanceIndex
	) {
		VkResult Result = VK_SUCCESS;
		// Get Mesh & Material Data from mesh instance.
		auto RasterizationPipeline = aCamera3D->Pipeline[0];
		auto MeshInstance = aObject->TotalMeshInstance[aMeshInstanceIndex];
		auto Mesh = aObject->Model->Mesh[MeshInstance->MeshIndex];
		auto Material = aObject->Model->Material[MeshInstance->MaterialIndex];
		auto Node = MeshInstance->Parent;
		// Get mesh instance world position center.
		math::vec<float, 3> MeshPosition = Node->GlobalTransform.minor(3,3) * math::vec<float, 3>(0.0f, 0.0f, 0.0f);
		// Get transparency mode for draw call data structure.
		this->TransparencyMode = (material::transparency)Material->UniformData.Transparency;
		// Set rendering priority.
		this->update(aCamera3D, aFrameIndex, aObject, aMeshInstanceIndex);
		// Load Context
		this->Context = aObject->Context;
		// Load up desired images which draw call will render to.
		std::vector<std::shared_ptr<image>> ImageOutputList = {
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Color"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Position"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Normal"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Tangent"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Bitangent"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.ORM"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Emissive"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.TranslucencyMask"],
			aCamera3D->Framechain->Image[aFrameIndex]["Depth"]
		};
		// Acquire Mesh Vertex Buffer, and Mesh Instance Vertex Weight Buffer.
		std::vector<std::shared_ptr<buffer>> VertexBuffer = { Mesh->VertexBuffer, MeshInstance->VertexWeightBuffer };
		// Load up GPU interface data to interface resources with pipeline.
		Framebuffer = Context->create_framebuffer(RasterizationPipeline, ImageOutputList, aCamera3D->Framechain->Resolution);
		DescriptorArray = Context->create_descriptor_array(RasterizationPipeline);
		DrawCommand = aCamera3D->CommandPool->allocate();

		// Bind Object Uniform Buffers
		DescriptorArray->bind(0, 0, 0, aCamera3D->SubjectUniformBuffer);			// Camera Position, Orientation, Projection
		DescriptorArray->bind(0, 1, 0, MeshInstance->UniformBuffer); 			// Mesh Instance Transform
		DescriptorArray->bind(0, 2, 0, Material->UniformBuffer); 				// Material Properties

		// Bind Material Textures.
		DescriptorArray->bind(1, 0, 0, Material->Texture["Albedo"]);
		DescriptorArray->bind(1, 1, 0, Material->Texture["Opacity"]);
		DescriptorArray->bind(1, 2, 0, Material->Texture["Normal"]);
		DescriptorArray->bind(1, 3, 0, Material->Texture["Height"]);
		DescriptorArray->bind(1, 4, 0, Material->Texture["Emissive"]);
		DescriptorArray->bind(1, 5, 0, Material->Texture["Specular"]);
		DescriptorArray->bind(1, 6, 0, Material->Texture["Shininess"]);
		DescriptorArray->bind(1, 7, 0, Material->Texture["AmbientOcclusion"]);
		DescriptorArray->bind(1, 8, 0, Material->Texture["Roughness"]);
		DescriptorArray->bind(1, 9, 0, Material->Texture["Metallic"]);
		// DescriptorArray->bind(1, 10, 0, Material->Texture["Sheen"]);
		// DescriptorArray->bind(1, 11, 0, Material->Texture["ClearCoat"]);

		// Actual Draw Call.
		Result = Context->begin(DrawCommand);
		// The goal of this barrier is to keep the gpu busy while still waiting on the depth results of the previous draw call.
		pipeline::barrier(DrawCommand, 
			/* Src ---> Dst */
			pipeline::stage::LATE_FRAGMENT_TESTS, 				pipeline::stage::EARLY_FRAGMENT_TESTS,
			device::access::DEPTH_STENCIL_ATTACHMENT_WRITE, 	device::access::DEPTH_STENCIL_ATTACHMENT_READ | device::access::DEPTH_STENCIL_ATTACHMENT_WRITE
		);
		// Issue Draw Call.
		RasterizationPipeline->draw(DrawCommand, Framebuffer, VertexBuffer, Mesh->IndexBuffer, DescriptorArray);
		Result = Context->end(DrawCommand);
	}

	void camera3d::rasterizer_draw_call::update(
		subject* 	aSubject, 
		size_t 		aFrameIndex,
		object* 	aObject, 
		size_t 		aMeshInstanceIndex
	) {
		camera3d* aCamera3D = (camera3d*)aSubject;
		auto MeshInstance = aObject->TotalMeshInstance[aMeshInstanceIndex];
		auto Mesh = aObject->Model->Mesh[MeshInstance->MeshIndex];
		auto Material = aObject->Model->Material[MeshInstance->MaterialIndex];
		auto Node = MeshInstance->Parent;
		math::mat<float, 3, 3> MeshTransform = Node->GlobalTransform.minor(3,3);
		// Transform Mesh Center of Mass to world space.
		math::vec<float, 3> MeshPosition = MeshTransform*Mesh->CenterOfMass;
		// Transform Vertex Extrema to world space.
		math::vec<float, 3> MeshBoundingRadius = MeshTransform*(Mesh->BoundingRadius + Mesh->CenterOfMass);
		// Calculate the distance from the camera to the mesh instance.
		float Distance = math::length(MeshPosition - aCamera3D->Position);
		// Calculate the world space radius of the mesh instance.
		float Radius = math::length(MeshBoundingRadius - MeshPosition);
		// TODO: Check if bounding radius is inside the camera frustum.
		// Calculate Rendering Priority.
		switch(this->TransparencyMode) {
		case material::transparency::OPAQUE:
			// Opaque objects nearest are rendered first.
			// Rendering Priority Parameters are as follows:
			// 1. How far mesh instance is away from the camera.
			// 2. How large the mesh instance is in the camera's view.
			// ! Make sure not to divide by zero.
			this->RenderingPriority = Radius * Radius / Distance;
			break;
		case material::transparency::TRANSPARENT:
			// Transparent objects, furthest are rendered first.
			this->RenderingPriority = Distance;
			break;
		case material::transparency::TRANSLUCENT:
			// Translucent objects, furthest are rendered first.
			this->RenderingPriority = Distance;
			break;
		default:
			break;
		}
	}

	camera3d::ray_trace_draw_call::ray_trace_draw_call(
		camera3d* 			aCamera3D,
		size_t 				aFrameIndex,
		runtime::stage* 	aStage
	) {
		VkResult Result = VK_SUCCESS;
		// Generate scene ray tracing here.
		auto RayTracingPipeline = aCamera3D->Pipeline[1];

		Context = aCamera3D->Context;
		DescriptorArray = Context->create_descriptor_array(RayTracingPipeline);
		DrawCommand = aCamera3D->CommandPool->allocate();

		// Bind Scene Geometry.
		DescriptorArray->bind(0, 0, 0, aStage->TLAS);
		// TODO: Load in globalized textures.
		DescriptorArray->bind(0, 13, 0, aStage->MaterialUniformBuffer);
		DescriptorArray->bind(0, 14, 0, aStage->LightUniformBuffer);
		// Bind Final Output Image.
		DescriptorArray->bind(1, 0, 0, aCamera3D->Framechain->Image[aFrameIndex]["Color"], image::layout::GENERAL);
		// Bind Geometry Buffer Images to Ray Tracing Pipeline.
		DescriptorArray->bind(1, 1, 0, aCamera3D->Framechain->Image[aFrameIndex]["OGB.Color"]);
		DescriptorArray->bind(1, 2, 0, aCamera3D->Framechain->Image[aFrameIndex]["OGB.Position"]);
		DescriptorArray->bind(1, 3, 0, aCamera3D->Framechain->Image[aFrameIndex]["OGB.Normal"]);
		DescriptorArray->bind(1, 4, 0, aCamera3D->Framechain->Image[aFrameIndex]["OGB.Tangent"]);
		DescriptorArray->bind(1, 5, 0, aCamera3D->Framechain->Image[aFrameIndex]["OGB.Bitangent"]);
		DescriptorArray->bind(1, 6, 0, aCamera3D->Framechain->Image[aFrameIndex]["OGB.ORM"]);
		DescriptorArray->bind(1, 7, 0, aCamera3D->Framechain->Image[aFrameIndex]["OGB.Emissive"]);
		DescriptorArray->bind(1, 8, 0, aCamera3D->Framechain->Image[aFrameIndex]["OGB.TranslucencyMask"]);
		// TODO: Include Translucency Mask for Ray Tracing.
		// Bind Camera Uniform Buffer.
		DescriptorArray->bind(2, 0, 0, aCamera3D->SubjectUniformBuffer);

		// Actual Draw Call.
		Result = Context->begin(DrawCommand);
		
		// Transition images to GENERAL layout for ray tracing storage access
		std::vector<std::shared_ptr<image>> ImagesToTransition = {
			aCamera3D->Framechain->Image[aFrameIndex]["Color"]
		};
		
		for (auto& img : ImagesToTransition) {
			img->transition(DrawCommand, image::layout::SHADER_READ_ONLY_OPTIMAL, image::layout::GENERAL, 
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		}
		
		// The goal of this barrier is to keep the gpu busy while still waiting on the depth results of the previous draw call.
		pipeline::barrier(DrawCommand, 
			/* Src ---> Dst */
			pipeline::stage::LATE_FRAGMENT_TESTS, 				pipeline::stage::EARLY_FRAGMENT_TESTS,
			device::access::DEPTH_STENCIL_ATTACHMENT_WRITE, 	device::access::DEPTH_STENCIL_ATTACHMENT_READ | device::access::DEPTH_STENCIL_ATTACHMENT_WRITE
		);
		
		// Issue Ray Tracing Call.
		RayTracingPipeline->raytrace(DrawCommand, DescriptorArray, aCamera3D->Framechain->Resolution);
		
		// Transition images back to SHADER_READ_ONLY_OPTIMAL for subsequent operations
		for (auto& img : ImagesToTransition) {
			img->transition(DrawCommand, image::layout::GENERAL, image::layout::SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}
		
		Result = Context->end(DrawCommand);
	}

	void camera3d::ray_trace_draw_call::update(
		subject* 			aSubject, 
		size_t 				aFrameIndex,
		runtime::stage* 	aStage
	) {}

	camera3d::stage_rasterizer::stage_rasterizer(
		camera3d* aCamera3D,
		object* aObject
	) : runtime::object::renderer(aCamera3D, aObject) {
		// Size is the number of frames times the number of mesh instances.
		this->DrawCallList = std::vector<std::vector<std::shared_ptr<draw_call>>>(aCamera3D->Framechain->Image.size(), std::vector<std::shared_ptr<draw_call>>(aObject->TotalMeshInstance.size()));
		// Generates draw calls for each frame in the frame chain and mesh instance.
		for (size_t i = 0; i < aCamera3D->Framechain->Image.size(); i++) {
			for (size_t j = 0; j < aObject->TotalMeshInstance.size(); j++) {
				DrawCallList[i][j] = geodesy::make<rasterizer_draw_call>(aCamera3D, i, aObject, j);
			}
		}
	}

	camera3d::stage_postprocessor::stage_postprocessor(
		camera3d* aCamera3D,
		runtime::stage* aStage
	) : runtime::object::renderer(aCamera3D, nullptr) {
		// Only need draw calls per frame in the frame chain.
		this->DrawCallList = std::vector<std::vector<std::shared_ptr<draw_call>>>(aCamera3D->Framechain->Image.size(), std::vector<std::shared_ptr<draw_call>>(1));
		for (size_t i = 0; i < aCamera3D->Framechain->Image.size(); i++) {
			// TODO: If the hardware supports ray tracing, use ray tracing for post processing.
			this->DrawCallList[i][0] = geodesy::make<ray_trace_draw_call>(aCamera3D, i, aStage);
			// Use standard deferred lighting for post processing if ray tracing is not supported.
			// this->DrawCallList[i][1] = geodesy::make<translucent_ray_trace_call>(aCamera3D, i, aStage);
		}
	}

	void camera3d::stage_postprocessor::update(
		double aDeltaTime,
		double aTime
	) {

	}

	camera3d::creator::creator() {
		this->RTTIID = camera3d::rttiid;
		this->FOV = 45.0f;
		this->Near = 0.1f;
		this->Far = 1000.0f;
	}

	camera3d::camera3d(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aCamera3DCreator) : runtime::subject(aContext, aStage, aCamera3DCreator) {
		VkResult Result = VK_SUCCESS;
		engine* Engine = aContext->Device->Engine;
		this->FOV 	= aCamera3DCreator->FOV;
		this->Near 	= aCamera3DCreator->Near;
		this->Far 	= aCamera3DCreator->Far;

		// List of assets Camera3D will load into memory.
		std::vector<std::string> AssetList = {
			// Standard Vertex Shader
			"dep/geodesy-src/assets/shader/standard.vert",
			// Opaque Rasterization
			"dep/geodesy-src/assets/shader/camera3d/opaque.frag",
			// Translucent Rasterization
			// "assets/shader/camera3d/translucent.frag",
			// Opaque Lighting & Shadows
			"dep/geodesy-src/assets/shader/camera3d/opaque.rgen",
			"dep/geodesy-src/assets/shader/camera3d/standard.rmiss",
			"dep/geodesy-src/assets/shader/camera3d/standard.rchit"
			// Translucent Renderings
			
			// Final Post Processing
		};

		// Loaded host memory assets for this object's possession.
		this->Asset = Engine->FileManager.open(AssetList);

		// Allocate GPU resources.
		this->Framechain = std::dynamic_pointer_cast<framechain>(std::make_shared<geometry_buffer>(aContext, aCamera3DCreator->Resolution, aCamera3DCreator->FrameRate, aCamera3DCreator->FrameCount));

		// Create GPU Pipelines for camera3d
		this->Pipeline = std::vector<std::shared_ptr<core::gpu::pipeline>>(2);
		this->Pipeline[0] = this->create_opaque_rasterizing_pipeline(aCamera3DCreator);
		this->Pipeline[1] = this->create_opaque_ray_tracing_pipeline(aCamera3DCreator);

		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		uniform_data UniformData = uniform_data(
			this->Position, 
			{ this->Theta, this->Phi },
			this->FOV, 
			this->Framechain->Resolution, 
			this->Near,
			this->Far
		);
		this->SubjectUniformBuffer = Context->create_buffer(UBCI, sizeof(UniformData), &UniformData);
		this->SubjectUniformBuffer->map_memory(0, sizeof(UniformData));
	}

	camera3d::~camera3d() {

	}

	void camera3d::input(const core::hid::input& aInputState) {
		float LinearSpeed = 7.5f;
		float RotationSpeed = 1.5f;
		float ForwardSpeed = 0.0f, RightSpeed = 0.0f;
		float DeltaTheta = 0.0f, DeltaPhi = 0.0f;
		
		// ----- WASD Control ----- //
		
		// Shift Speed Boost
		if (aInputState.Keyboard[hid::keyboard::KEY_LEFT_SHIFT]) LinearSpeed 	*= 2.5f;
		if (aInputState.Keyboard[hid::keyboard::KEY_W]) ForwardSpeed 			+= LinearSpeed;
		if (aInputState.Keyboard[hid::keyboard::KEY_S]) ForwardSpeed 			-= LinearSpeed;
		if (aInputState.Keyboard[hid::keyboard::KEY_A]) RightSpeed 				-= LinearSpeed;
		if (aInputState.Keyboard[hid::keyboard::KEY_D]) RightSpeed 				+= LinearSpeed;

		// ----- Mouse Control ----- //

		// TODO: Lock look angles to prevent gimbal lock.
		if (!std::isinf(aInputState.Mouse.Velocity[1]) && !std::isnan(aInputState.Mouse.Velocity[1])) {
			this->Theta 	+= math::radians(aInputState.Mouse.Velocity[1]) * RotationSpeed;
		}
		if (!std::isinf(aInputState.Mouse.Velocity[0]) && !std::isnan(aInputState.Mouse.Velocity[0])) {
			this->Phi 		-= math::radians(aInputState.Mouse.Velocity[0]) * RotationSpeed;
		}
		if (this->Theta > math::constant::pi) this->Theta = math::constant::pi;
		if (this->Theta < 0) this->Theta = 0;

		math::mat<float, 3, 3> OrientationMatrix = math::mat<float, 4, 4>(math::orientation(this->Theta, this->Phi)).minor(3,3);
		math::vec<float, 3> DirectionRight = OrientationMatrix * math::vec<float, 3>(1.0f, 0.0f, 0.0f);
		math::vec<float, 3> DirectionForward = OrientationMatrix * math::vec<float, 3>(0.0f, 1.0f, 0.0f);
		math::vec<float, 3> DirectionUp = OrientationMatrix * math::vec<float, 3>(0.0f, 0.0f, 1.0f);

		this->InputVelocity = DirectionForward * ForwardSpeed + DirectionRight * RightSpeed;
	}

	void camera3d::host_update(
		double 										aDeltaTime, 
		double 										aTime, 
		const std::vector<core::phys::force>& 		aAppliedForces,
		const std::vector<core::phys::animation>& 	aPlaybackAnimation,
		const std::vector<float>& 					aAnimationWeight
	) {

		object::host_update(aDeltaTime, aTime, aAppliedForces);

		// How the object will move according to its current momentum.
		this->LinearMomentum += (this->InputForce) * aDeltaTime;
		this->Position += (this->LinearMomentum / this->Mass + this->InputVelocity) * aDeltaTime;
	}

	void camera3d::device_update(
		double 											DeltaTime, 
		double 											Time, 
		const std::vector<core::phys::force>& 			AppliedForces
	) {

		object::device_update(DeltaTime, Time, AppliedForces);

		*(uniform_data*)this->SubjectUniformBuffer->Ptr = uniform_data(
			this->Position, 
			{ this->Theta, this->Phi },
			this->FOV, 
			this->Framechain->Resolution, 
			this->Near,
			this->Far
		);
	}

	std::shared_ptr<runtime::object::renderer> camera3d::default_renderer(object* aObject) {
		return std::dynamic_pointer_cast<runtime::object::renderer>(geodesy::make<stage_rasterizer>(this, aObject));
	}

	std::shared_ptr<runtime::object::renderer> camera3d::default_post_processor(runtime::stage* aStage) {
		return std::dynamic_pointer_cast<runtime::object::renderer>(geodesy::make<stage_postprocessor>(this, aStage));
	}

	// std::shared_ptr<runtime::object::renderer> opaque_raytracer(runtime::stage* aStage) {
	// 	return std::dynamic_pointer_cast<runtime::object::renderer>(geodesy::make<camera3d::opaque_raytracer>(aStage));
	// }

	// ----- Geodesy Default 3D Real Time Rendering System ----- //
	// this->RenderingOperations[0]: Clears all images in geometry buffer.
	// this->RenderingOperations[1]: Opaque Mesh Instance Renderings
	// this->RenderingOperations[2]: Transparent Mesh Instance Renderings
	// this->RenderingOperations[3]: Translucent Mesh Instance Renderings generate ray casting data for ray tracer.
	// this->RenderingOperations[4]: Lighting & Shadows on Opaque Geometry Buffer
	// this->RenderingOperations[5]: Ray Tracing informed by Translucent Outputs.
	// this->RenderingOperations[6]: Post Processing to Final Color Output.
	// this->RenderingOperations[7]: postdraw operations.
	///*
	core::gpu::submission_batch camera3d::render(runtime::stage* aStage) {

		// Get next frame.
		VkResult Result = this->Framechain->next_frame();

		// Predraw operations.
		this->RenderingOperations += this->Framechain->predraw();

		// Collect all draw calls and separate by type in a single pass
		std::vector<std::vector<std::shared_ptr<draw_call>>> AllDrawCalls(aStage->Object.size());

		for (size_t i = 0; i < aStage->Object.size(); i++) {
			AllDrawCalls[i] = aStage->Object[i]->draw(this);
		}

		// Count draw calls by type for optimal memory allocation
		size_t OpaqueCount = 0, TransparentCount = 0, TranslucentCount = 0;
		for (size_t i = 0; i < AllDrawCalls.size(); i++) {
			for (size_t j = 0; j < AllDrawCalls[i].size(); j++) {
				switch(AllDrawCalls[i][j]->TransparencyMode) {
				case gfx::material::transparency::OPAQUE:
					OpaqueCount++;
					break;
				case gfx::material::transparency::TRANSPARENT:
					TransparentCount++;
					break;
				case gfx::material::transparency::TRANSLUCENT:
					TranslucentCount++;
					break;
				default:
					break;
				}
			}
		}

		// Pre-allocate vectors with exact sizes to avoid reallocations
		std::vector<std::shared_ptr<draw_call>> OpaqueVector(OpaqueCount);
		std::vector<std::shared_ptr<draw_call>> TransparentVector(TransparentCount);
		std::vector<std::shared_ptr<draw_call>> TranslucentVector(TranslucentCount);

		// Separate draw calls by type using indexed assignment
		size_t OpaqueIndex = 0, TransparentIndex = 0, TranslucentIndex = 0;
		for (size_t i = 0; i < AllDrawCalls.size(); i++) {
			for (size_t j = 0; j < AllDrawCalls[i].size(); j++) {
				switch(AllDrawCalls[i][j]->TransparencyMode) {
				case gfx::material::transparency::OPAQUE:
					OpaqueVector[OpaqueIndex++] = AllDrawCalls[i][j];
					break;
				case gfx::material::transparency::TRANSPARENT:
					TransparentVector[TransparentIndex++] = AllDrawCalls[i][j];
					break;
				case gfx::material::transparency::TRANSLUCENT:
					TranslucentVector[TranslucentIndex++] = AllDrawCalls[i][j];
					break;
				default:
					break;
				}
			}
		}

		// Sort all vectors efficiently: O(n log n) instead of O(n²)
		// Opaque: nearest first (ascending distance)
		std::sort(OpaqueVector.begin(), OpaqueVector.end(), 
			[](const auto& a, const auto& b) {
				return a->RenderingPriority > b->RenderingPriority;
			});

		// Transparent/Translucent: farthest first (descending distance)  
		std::sort(TransparentVector.begin(), TransparentVector.end(),
			[](const auto& a, const auto& b) {
				return a->RenderingPriority > b->RenderingPriority;
			});

		std::sort(TranslucentVector.begin(), TranslucentVector.end(),
			[](const auto& a, const auto& b) {
				return a->RenderingPriority > b->RenderingPriority;
			});

		// Add to Rendering Operations.
		this->RenderingOperations += command_batch(convert(OpaqueVector));
		this->RenderingOperations += command_batch(convert(TransparentVector));
		this->RenderingOperations += command_batch(convert(TranslucentVector));
		// Ray Tracing.
		this->RenderingOperations += command_batch(convert(aStage->post_processing(this)));
		// this->RenderingOperations += aStage->draw(this);
		// Ray Tracing Operations on Translucent Geometry Buffer.

		// Post Processing Operations on Final Color Output.
		this->RenderingOperations += this->Framechain->postdraw();

		// Setup Semaphore dependencies between command_batches.
		for (size_t i = 0; i < this->RenderingOperations.size() - 1; i++) {
			VkPipelineStageFlags Stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			this->RenderingOperations[i + 1].depends_on(this->SemaphorePool, Stage, this->RenderingOperations[i]);
		}

		// Build submission batch and prepare for GPU execution.
		return build(this->RenderingOperations);
	}
	//*/

	std::shared_ptr<core::gpu::pipeline> camera3d::create_opaque_rasterizing_pipeline(creator* aCamera3DCreator) {
		// Grab shaders from asset list, compile, and link.
		std::shared_ptr<gpu::shader> VertexShader = std::dynamic_pointer_cast<gpu::shader>(Asset[0]);
		std::shared_ptr<gpu::shader> PixelShader = std::dynamic_pointer_cast<gpu::shader>(Asset[1]);
		std::vector<std::shared_ptr<gpu::shader>> ShaderList = { VertexShader, PixelShader };
		std::shared_ptr<pipeline::rasterizer> Rasterizer = geodesy::make<pipeline::rasterizer>(ShaderList, aCamera3DCreator->Resolution);

		// How to intepret vertex data in rasterization.
		Rasterizer->InputAssembly.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		Rasterizer->InputAssembly.primitiveRestartEnable	= false;

		// Rasterizer Info
		Rasterizer->Rasterizer.rasterizerDiscardEnable		= VK_FALSE;
		Rasterizer->Rasterizer.polygonMode					= VK_POLYGON_MODE_FILL;
		Rasterizer->Rasterizer.cullMode						= VK_CULL_MODE_BACK_BIT;
		Rasterizer->Rasterizer.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// Copy Paste
		Rasterizer->Multisample.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;

		// Oncoming Depth Value [OPERATOR] Depth Value In Buffer
		// Needed for 3D graphics.
		Rasterizer->DepthStencil.depthTestEnable			= VK_TRUE;
		Rasterizer->DepthStencil.depthWriteEnable			= VK_TRUE;
		Rasterizer->DepthStencil.depthCompareOp				= VK_COMPARE_OP_GREATER; // Camera, +z is closer.
		Rasterizer->DepthStencil.minDepthBounds				= 0.0f;
		Rasterizer->DepthStencil.maxDepthBounds				= 1.0f;

		// Color Blending operations.
		std::vector<VkPipelineColorBlendAttachmentState> AlphaBlendOperation(Rasterizer->ColorAttachment.size());
		for (auto& AB : AlphaBlendOperation) {
			AB.blendEnable = VK_TRUE;
			AB.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			// Set to direct overwrite (not used since blendEnable = false, but good practice)
			AB.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			AB.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			AB.colorBlendOp = VK_BLEND_OP_ADD;
			AB.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			AB.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			AB.alphaBlendOp = VK_BLEND_OP_ADD;
		}

		Rasterizer->ColorBlend.logicOpEnable 		= VK_FALSE;
		Rasterizer->ColorBlend.logicOp 				= VK_LOGIC_OP_COPY;
		Rasterizer->ColorBlend.attachmentCount 		= AlphaBlendOperation.size();
		Rasterizer->ColorBlend.pAttachments 		= AlphaBlendOperation.data();

		// This code specifies how the vextex data is to be interpreted from the bound vertex buffers.
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 0, offsetof(gfx::mesh::vertex, Position));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 1, offsetof(gfx::mesh::vertex, Normal));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 2, offsetof(gfx::mesh::vertex, Tangent));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 3, offsetof(gfx::mesh::vertex, Bitangent));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 4, offsetof(gfx::mesh::vertex, TextureCoordinate));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 5, offsetof(gfx::mesh::vertex, Color));
		Rasterizer->bind(1, sizeof(gfx::mesh::vertex::weight), 6, offsetof(gfx::mesh::vertex::weight, BoneID));
		Rasterizer->bind(1, sizeof(gfx::mesh::vertex::weight), 7, offsetof(gfx::mesh::vertex::weight, BoneWeight));
		
		// Select output attachments for pipeline.
		Rasterizer->attach(0, this->Framechain->draw_frame()["OGB.Color"]);
		Rasterizer->attach(1, this->Framechain->draw_frame()["OGB.Position"]);
		Rasterizer->attach(2, this->Framechain->draw_frame()["OGB.Normal"]);
		Rasterizer->attach(3, this->Framechain->draw_frame()["OGB.Tangent"]);
		Rasterizer->attach(4, this->Framechain->draw_frame()["OGB.Bitangent"]);
		Rasterizer->attach(5, this->Framechain->draw_frame()["OGB.ORM"]);
		Rasterizer->attach(6, this->Framechain->draw_frame()["OGB.Emissive"]);
		Rasterizer->attach(7, this->Framechain->draw_frame()["OGB.TranslucencyMask"]);
		Rasterizer->attach(8, this->Framechain->draw_frame()["Depth"]);

		// Create render pipeline for camera3d.
		return Context->create_pipeline(Rasterizer);
	}

	std::shared_ptr<core::gpu::pipeline> camera3d::create_opaque_ray_tracing_pipeline(creator* aCamera3DCreator) {
		// Stack Space
		auto RayGenerationShader = std::dynamic_pointer_cast<gpu::shader>(Asset[2]);
		auto MissShader = std::dynamic_pointer_cast<gpu::shader>(Asset[3]);
		auto HitShader = std::dynamic_pointer_cast<gpu::shader>(Asset[4]);
		
		std::vector<gpu::pipeline::raytracer::shader_group> ShaderGroup;
		// Ray Generation Shader
		ShaderGroup.push_back({ RayGenerationShader, nullptr, nullptr, nullptr, });
		// Miss Shader
		ShaderGroup.push_back({ MissShader, nullptr, nullptr, nullptr, });
		// Closest Hit Shader
		ShaderGroup.push_back({ nullptr, HitShader,  nullptr, nullptr, });
		// Generate host ray tracer.
		std::shared_ptr<gpu::pipeline::raytracer> RayTracer = geodesy::make<pipeline::raytracer>(ShaderGroup, 1u);
		// Create GPU Ray Tracing Pipeline.
		return Context->create_pipeline(RayTracer);
	}

}
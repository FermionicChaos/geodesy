#include <geodesy/bltn/obj/camera3d.h>

#include <iostream>

namespace geodesy::bltn::obj {

	using namespace geodesy::core;
	using namespace geodesy::core::gfx;
	using namespace geodesy::core::gpu;

	camera3d::uniform_data::uniform_data(
		math::vec<float, 3> aPosition, 
		math::vec<float, 3> aDirRight,
		math::vec<float, 3> aDirUp,
		math::vec<float, 3> aDirForward,
		float aFOV,
		math::vec<uint, 3> aResolution,
		float aNear,
		float aFar
	) {
		float AspectRatio = static_cast<float>(aResolution[0]) / static_cast<float>(aResolution[1]);
		this->Position = aPosition;
		this->Rotation = math::mat<float, 4, 4>(
			 aDirRight[0], 		 aDirRight[1], 		 aDirRight[2], 			0.0f,
			-aDirUp[0], 		-aDirUp[1], 		-aDirUp[2], 			0.0f,
			 aDirForward[0], 	 aDirForward[1], 	 aDirForward[2], 		0.0f,
			 0.0f, 				 0.0f, 				 0.0f, 					1.0f
		);
		this->Projection = math::perspective(math::radians(aFOV), AspectRatio, aNear, aFar);
	}

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
		DepthCreateInfo.Layout		= image::layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		DepthCreateInfo.Memory		= device::memory::DEVICE_LOCAL;
		DepthCreateInfo.Usage		= image::usage::SAMPLED | image::usage::DEPTH_STENCIL_ATTACHMENT  | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		image::create_info ColorCreateInfo;
		ColorCreateInfo.Layout		= image::layout::SHADER_READ_ONLY_OPTIMAL;
		ColorCreateInfo.Memory		= device::memory::DEVICE_LOCAL;
		ColorCreateInfo.Usage		= image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		image::format ColorFormat = image::format::R32G32B32A32_SFLOAT;
		image::format DepthFormat = image::format::D32_SFLOAT;

		this->Resolution = aResolution;
		for (std::size_t i = 0; i < this->Image.size(); i++) {
			// This is the finalized color output.
			this->Image[i]["Color"] 		= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
			// Opaque Geometry Buffer, used for Opaque and Transparent mesh instances.
			this->Image[i]["OGB.Color"] 	= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Position"] 	= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Normal"] 	= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Emissive"] 	= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.SS"] 		= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.ORM"] 		= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Depth"] 	= aContext->create_image(DepthCreateInfo, DepthFormat, aResolution[0], aResolution[1]);
			// Outputs for post processing.
		}

		// Setup frame clearing commands.
		this->PredrawFrameOperation = std::vector<command_batch>(this->Image.size());
		this->PostdrawFrameOperation = std::vector<command_batch>(this->Image.size());
		for (size_t i = 0; i < this->Image.size(); i++) {
			// Creates Clearing command for images in frame chain.
			VkCommandBuffer	ClearCommand = aContext->allocate_command_buffer(device::operation::GRAPHICS_AND_COMPUTE);
			aContext->begin(ClearCommand);
			// this->Image[i]["Color"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Color"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Position"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Normal"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Emissive"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.SS"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.ORM"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			this->Image[i]["OGB.Depth"]->clear_depth(ClearCommand, { 1.0f, 0 }, image::layout::DEPTH_ATTACHMENT_OPTIMAL);
			// this->Image[i]["FinalColor"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
			aContext->end(ClearCommand);
			this->PredrawFrameOperation[i] += ClearCommand;
		}

	}

	camera3d::deferred_draw_call::deferred_draw_call(
		object* 						aObject, 
		core::gfx::mesh::instance* 		aMeshInstance,
		camera3d* 						aCamera3D,
		size_t 							aFrameIndex
	) {
		VkResult Result = VK_SUCCESS;
		// Get distance from subject.
		// TODO: We need to calculate center of mass for mesh instance using node hierarchy.
		this->DistanceFromSubject = math::length(aObject->Position - aCamera3D->Position);
		// Get transparency mode for draw call data structure.
		this->TransparencyMode = (material::transparency)aObject->Model->Material[aMeshInstance->MaterialIndex]->UniformData.Transparency;
		// Load Context
		this->Context = aObject->Context;
		// Get Mesh & Material Data from mesh instance.
		std::shared_ptr<mesh> Mesh = aObject->Model->Mesh[aMeshInstance->Index];
		std::shared_ptr<material> Material = aObject->Model->Material[aMeshInstance->MaterialIndex];
		// Load up desired images which draw call will render to.
		std::vector<std::shared_ptr<image>> ImageOutputList = {
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Color"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Position"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Normal"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Emissive"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.SS"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.ORM"],
			aCamera3D->Framechain->Image[aFrameIndex]["OGB.Depth"]
		};
		// Acquire Mesh Vertex Buffer, and Mesh Instance Vertex Weight Buffer.
		std::vector<std::shared_ptr<buffer>> VertexBuffer = { Mesh->VertexBuffer, aMeshInstance->VertexWeightBuffer };
		// Load up GPU interface data to interface resources with pipeline.
		Framebuffer = Context->create_framebuffer(aCamera3D->Pipeline, ImageOutputList, aCamera3D->Framechain->Resolution);
		DescriptorArray = Context->create_descriptor_array(aCamera3D->Pipeline);
		DrawCommand = aCamera3D->CommandPool->allocate();

		// Bind Object Uniform Buffers
		DescriptorArray->bind(0, 0, 0, aCamera3D->CameraUniformBuffer);			// Camera Position, Orientation, Projection
		DescriptorArray->bind(0, 1, 0, aObject->UniformBuffer);					// Object Position, Orientation, Scale
		DescriptorArray->bind(0, 2, 0, aMeshInstance->UniformBuffer); 			// Mesh Instance Transform
		DescriptorArray->bind(0, 3, 0, Material->UniformBuffer); 				// Material Properties

		// Bind Material Textures.
		DescriptorArray->bind(1, 0, 0, Material->Texture["Color"]);
		DescriptorArray->bind(1, 1, 0, Material->Texture["Opacity"]);
		DescriptorArray->bind(1, 2, 0, Material->Texture["Normal"]);
		DescriptorArray->bind(1, 3, 0, Material->Texture["Height"]);
		DescriptorArray->bind(1, 4, 0, Material->Texture["Emissive"]);
		DescriptorArray->bind(1, 5, 0, Material->Texture["Specular"]);
		DescriptorArray->bind(1, 6, 0, Material->Texture["Shininess"]);
		DescriptorArray->bind(1, 7, 0, Material->Texture["AmbientOcclusion"]);
		DescriptorArray->bind(1, 8, 0, Material->Texture["Metallic"]);
		DescriptorArray->bind(1, 9, 0, Material->Texture["Roughness"]);
		DescriptorArray->bind(1, 10, 0, Material->Texture["Sheen"]);
		DescriptorArray->bind(1, 11, 0, Material->Texture["ClearCoat"]);

		// Actual Draw Call.
		Result = Context->begin(DrawCommand);
		aCamera3D->Pipeline->draw(DrawCommand, Framebuffer, VertexBuffer, Mesh->IndexBuffer, DescriptorArray);
		Result = Context->end(DrawCommand);
	}

	camera3d::deferred_renderer::deferred_renderer(object* aObject, camera3d* aCamera3D) : ecs::object::renderer(aObject, aCamera3D) {
		// Gather list of mesh instances throughout model hierarchy.
		std::vector<mesh::instance*> MeshInstance = aObject->Model->Hierarchy.gather_mesh_instances();

		this->DrawCallList = std::vector<std::vector<std::shared_ptr<draw_call>>>(aCamera3D->Framechain->Image.size(), std::vector<std::shared_ptr<draw_call>>(MeshInstance.size()));

		// Generates draw calls for each frame in the frame chain and mesh instance.
		for (size_t i = 0; i < aCamera3D->Framechain->Image.size(); i++) {
			for (size_t j = 0; j < MeshInstance.size(); j++) {
				DrawCallList[i][j] = geodesy::make<deferred_draw_call>(aObject, MeshInstance[j], aCamera3D, i);
			}
		}
	}

	camera3d::camera3d(std::shared_ptr<core::gpu::context> aContext, ecs::stage* aStage, creator* aCamera3DCreator) : ecs::subject(aContext, aStage, aCamera3DCreator) {
		VkResult Result = VK_SUCCESS;
		engine* Engine = aContext->Device->Engine;
		this->FOV 	= aCamera3DCreator->FOV;
		this->Near 	= aCamera3DCreator->Near;
		this->Far 	= aCamera3DCreator->Far;

		// List of assets Camera3D will load into memory.
		std::vector<std::string> AssetList = {
			// Standard Vertex Shader
			"assets/shader/camera3d/standard.vert",
			// Opaque Rasterization
			"assets/shader/camera3d/opaque.frag",
			// Transulucent Rasterization
			// Opaque Lighting & Shadows
			"assets/shader/camera3d/opaque.rgen",
			"assets/shader/camera3d/opaque.rmiss",
			"assets/shader/camera3d/opaque.rchit"
			// Translucent Renderings
			// Final Post Processing
		};

		// Loaded host memory assets for this object's possession.
		this->Asset = Engine->FileManager.open(AssetList);

		// Allocate GPU resources.
		this->Framechain = std::dynamic_pointer_cast<framechain>(std::make_shared<geometry_buffer>(aContext, aCamera3DCreator->Resolution, aCamera3DCreator->FrameRate, aCamera3DCreator->FrameCount));

		// Grab shaders from asset list, compile, and link.
		std::shared_ptr<gpu::shader> VertexShader = std::dynamic_pointer_cast<gpu::shader>(Asset[0]);
		std::shared_ptr<gpu::shader> PixelShader = std::dynamic_pointer_cast<gpu::shader>(Asset[1]);
		std::vector<std::shared_ptr<gpu::shader>> ShaderList = { VertexShader, PixelShader };
		std::shared_ptr<pipeline::rasterizer> Rasterizer = std::make_shared<pipeline::rasterizer>(ShaderList, aCamera3DCreator->Resolution);

		// This code specifies how the vextex data is to be interpreted from the bound vertex buffers.
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 0, offsetof(gfx::mesh::vertex, Position));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 1, offsetof(gfx::mesh::vertex, Normal));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 2, offsetof(gfx::mesh::vertex, Tangent));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 3, offsetof(gfx::mesh::vertex, Bitangent));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 4, offsetof(gfx::mesh::vertex, TextureCoordinate));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 5, offsetof(gfx::mesh::vertex, Color));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 1, sizeof(gfx::mesh::vertex::weight), 6, offsetof(gfx::mesh::vertex::weight, BoneID));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 1, sizeof(gfx::mesh::vertex::weight), 7, offsetof(gfx::mesh::vertex::weight, BoneWeight));
		
		// Select output attachments for pipeline.
		Rasterizer->attach(0, this->Framechain->draw_frame()["OGB.Color"], 		image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(1, this->Framechain->draw_frame()["OGB.Position"], 	image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(2, this->Framechain->draw_frame()["OGB.Normal"], 	image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(3, this->Framechain->draw_frame()["OGB.Emissive"], 	image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(4, this->Framechain->draw_frame()["OGB.SS"], 		image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(5, this->Framechain->draw_frame()["OGB.ORM"], 		image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(6, this->Framechain->draw_frame()["OGB.Depth"], 		image::layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

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
		Rasterizer->DepthStencil.depthCompareOp				= VK_COMPARE_OP_LESS; // Camera, +z is closer.
		Rasterizer->DepthStencil.minDepthBounds				= 0.0f;
		Rasterizer->DepthStencil.maxDepthBounds				= 1.0f;

		// Create render pipeline for camera3d.
		this->Pipeline = Context->create_pipeline(Rasterizer);

		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		uniform_data UniformData = uniform_data(
			this->Position, 
			this->DirectionRight, 
			this->DirectionUp, 
			this->DirectionFront, 
			this->FOV, 
			this->Framechain->Resolution, 
			this->Near,
			this->Far
		);
		this->CameraUniformBuffer = Context->create_buffer(UBCI, sizeof(UniformData), &UniformData);
		this->CameraUniformBuffer->map_memory(0, sizeof(UniformData));
	}

	camera3d::~camera3d() {

	}

	void camera3d::input(const core::hid::input& aInputState) {
		float LinearSpeed = 250.0f;
		float RotationSpeed = 1.5f;
		float ForwardSpeed = 0.0f, RightSpeed = 0.0f;		
		float DeltaTheta = 0.0f, DeltaPhi = 0.0f;

		if (aInputState.Keyboard[hid::keyboard::KEY_W]) ForwardSpeed 	+= LinearSpeed;
		if (aInputState.Keyboard[hid::keyboard::KEY_S]) ForwardSpeed 	-= LinearSpeed;
		if (aInputState.Keyboard[hid::keyboard::KEY_A]) RightSpeed 		-= LinearSpeed;
		if (aInputState.Keyboard[hid::keyboard::KEY_D]) RightSpeed 		+= LinearSpeed;

		// TODO: Lock look angles to prevent gimbal lock.
		this->Theta 	+= math::radians(aInputState.Mouse.Velocity[1]) * RotationSpeed;
		this->Phi 		-= math::radians(aInputState.Mouse.Velocity[0]) * RotationSpeed;
		if (this->Theta > math::constant::pi) this->Theta = math::constant::pi;
		if (this->Theta < 0) this->Theta = 0;
		this->InputVelocity = this->DirectionFront * ForwardSpeed + this->DirectionRight * RightSpeed;

	}

	void camera3d::update(double aDeltaTime, core::math::vec<float, 3> aAppliedForce, core::math::vec<float, 3> aAppliedTorque) {
		// How the object will move according to its current momentum.
		this->LinearMomentum += (aAppliedForce + this->InputForce) * aDeltaTime;
		this->Position += (this->LinearMomentum / this->Mass + this->InputVelocity) * aDeltaTime;

		this->DirectionRight			= {  std::sin(Phi), 					-std::cos(Phi), 					0.0f 			};
		this->DirectionUp				= { -std::cos(Theta) * std::cos(Phi), 	-std::cos(Theta) * std::sin(Phi), 	std::sin(Theta) };
		this->DirectionFront			= {  std::sin(Theta) * std::cos(Phi), 	 std::sin(Theta) * std::sin(Phi), 	std::cos(Theta) };

		*(uniform_data*)this->CameraUniformBuffer->Ptr = uniform_data(
			this->Position, 
			this->DirectionRight, 
			this->DirectionUp, 
			this->DirectionFront, 
			this->FOV, 
			this->Framechain->Resolution, 
			this->Near,
			this->Far
		);
	}

	std::shared_ptr<ecs::object::renderer> camera3d::default_renderer(object* aObject) {
		return std::dynamic_pointer_cast<ecs::object::renderer>(std::make_shared<deferred_renderer>(aObject, this));
	}

	// ----- Geodesy Default 3D Real Time Rendering System ----- //
	// this->RenderingOperations[0]: Clears all images in geometry buffer.
	// this->RenderingOperations[1]: Opaque Mesh Instance Renderings
	// this->RenderingOperations[2]: Transparent Mesh Instance Renderings
	// this->RenderingOperations[3]: Translucent Mesh Instance Renderings for ray tracing.
	// this->RenderingOperations[4]: Lighting & Shadows on Opaque Geometry Buffer
	// this->RenderingOperations[5]: Ray Tracing informed by Translucent Outputs.
	// this->RenderingOperations[6]: Post Processing to Final Color Output.
	// this->RenderingOperations[7]: postdraw operations.
	/*
	core::gpu::submission_batch camera3d::render(ecs::stage* aStage) {

		// Get next frame.
		VkResult Result = this->Framechain->next_frame();

		// Predraw operations.
		this->RenderingOperations += this->Framechain->predraw();

		std::list<std::shared_ptr<draw_call>> OpaqueList;
		std::list<std::shared_ptr<draw_call>> TransparentList;
		std::list<std::shared_ptr<draw_call>> TranslucentList;

		// Iterate through all draw calls and sort based on assigned material to mesh instance.
		for (auto& Object : aStage->Object) {
			std::vector<std::shared_ptr<draw_call>> DrawCallList = Object->draw(this);
			for (auto& DrawCall : DrawCallList) {
				// Use insert sort to sort draw calls by distance from camera.
				switch(DrawCall->TransparencyMode) {
					case gfx::material::transparency::OPAQUE: {
							// Sort nearest to the camera first.
							// insert 2.0
							// 0.2 | 0.7 | 3.3 | 4.5
							// Rewrite code, when list empty, push back, and fix if larger than all elements.
							bool Inserted = false;
							for (auto it = OpaqueList.begin(); it != OpaqueList.end(); ++it) {
								if (DrawCall->DistanceFromSubject < (*it)->DistanceFromSubject) {
									OpaqueList.insert(it, DrawCall);
									Inserted = true;
									break;
								}
							}
							// If not inserted, push back.
							if (!Inserted) {
								OpaqueList.push_back(DrawCall);
							}
						}
						break;
					case gfx::material::transparency::TRANSPARENT: {
							// This section sorts by farthest to the camera first.
							// insert 2.0
							// 4.5 | 3.3 | 0.7 | 0.2
							// Copy the opaque code but in reverse order.
							bool Inserted = false;
							for (auto it = TransparentList.begin(); it != TransparentList.end(); ++it) {
								if (DrawCall->DistanceFromSubject > (*it)->DistanceFromSubject) {
									TransparentList.insert(it, DrawCall);
									Inserted = true;
									break;
								}
							}
							// If not inserted, push back.
							if (!Inserted) {
								TransparentList.push_back(DrawCall);
							}
						}
						break;
					case gfx::material::transparency::TRANSLUCENT: {
							// This section sorts by farthest to the camera first.
							// insert 2.0
							// 4.5 | 3.3 | 0.7 | 0.2
							// Repeat the transparent code.
							bool Inserted = false;
							for (auto it = TranslucentList.begin(); it != TranslucentList.end(); ++it) {
								if (DrawCall->DistanceFromSubject > (*it)->DistanceFromSubject) {
									TranslucentList.insert(it, DrawCall);
									Inserted = true;
									break;
								}
							}
							// If not inserted, push back.
							if (!Inserted) {
								TranslucentList.push_back(DrawCall);
							}
						}
						break;
					default:
						break;
				}
			}
		}

		// Convert Linked List objects into command batches.
		{
			std::vector<std::shared_ptr<draw_call>> OpaqueVector(OpaqueList.begin(), OpaqueList.end());
			std::vector<std::shared_ptr<draw_call>> TransparentVector(TransparentList.begin(), TransparentList.end());
			std::vector<std::shared_ptr<draw_call>> TranslucentVector(TranslucentList.begin(), TranslucentList.end());
			// Convert to Command Buffer arrays.
			std::vector<VkCommandBuffer> OpaqueCommandBuffer = convert(OpaqueVector);
			std::vector<VkCommandBuffer> TransparentCommandBuffer = convert(TransparentVector);
			std::vector<VkCommandBuffer> TranslucentCommandBuffer = convert(TranslucentVector);
			// Convert to Command Batches.
			command_batch OpaqueBatch(OpaqueCommandBuffer);
			command_batch TransparentBatch(TransparentCommandBuffer);
			command_batch TranslucentBatch(TranslucentCommandBuffer);
			// Add to Rendering Operations.
			this->RenderingOperations += OpaqueBatch;
			this->RenderingOperations += TransparentBatch;
			this->RenderingOperations += TranslucentBatch;
		}

		// Shadow & Lighting Operations on Opaque Geometry Buffer.

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

}
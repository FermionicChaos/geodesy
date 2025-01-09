#include <geodesy/bltn/obj/camera3d.h>

#include <iostream>

namespace geodesy::bltn::obj {

	using namespace geodesy::core;
	using namespace geodesy::core::gfx;
	using namespace geodesy::core::gcl;

	namespace {

		struct camera_uniform_data {
			alignas(16) math::vec<float, 3> Position;
			alignas(16) math::mat<float, 4, 4> Rotation;
			alignas(16) math::mat<float, 4, 4> Projection;
			camera_uniform_data(
				math::vec<float, 3> aPosition, 
				math::vec<float, 3> aDirRight,
				math::vec<float, 3> aDirUp,
				math::vec<float, 3> aDirForward,
				float aFOV,
				math::vec<uint, 3> aResolution,
				float aNear,
				float aFar
			);
		};

		camera_uniform_data::camera_uniform_data(
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

		class geometry_buffer : public framechain {
		public:
			geometry_buffer(std::shared_ptr<context> aContext, math::vec<uint, 3> aResolution, double aFrameRate, size_t aFrameCount);
		};

		geometry_buffer::geometry_buffer(
			std::shared_ptr<context> aContext, 
			math::vec<uint, 3> aResolution, 
			double aFrameRate, 
			size_t aFrameCount
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
				this->Image[i]["OGB.Depth"] 	= aContext->create_image(DepthCreateInfo, DepthFormat, aResolution[0], aResolution[1]);
				this->Image[i]["FinalColor"] 	= aContext->create_image(ColorCreateInfo, ColorFormat, aResolution[0], aResolution[1]);
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
				this->Image[i]["OGB.Depth"]->clear_depth(ClearCommand, { 1.0f, 0 }, image::layout::DEPTH_ATTACHMENT_OPTIMAL);
				// this->Image[i]["FinalColor"]->clear(ClearCommand, { 0.0f, 0.0f, 0.0f, 1.0f });
				aContext->end(ClearCommand);
				this->PredrawFrameOperation[i] += ClearCommand;
			}

		}

	}


	camera3d::camera3d(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, creator* aCamera3DCreator) : ecs::subject(aContext, aStage, aCamera3DCreator) {
		VkResult Result = VK_SUCCESS;
		engine* Engine = aContext->Device->Engine;
		this->FOV 	= aCamera3DCreator->FOV;
		this->Near 	= aCamera3DCreator->Near;
		this->Far 	= aCamera3DCreator->Far;

		// List of assets Camera3D will load into memory.
		std::vector<std::string> AssetList = {
			"assets/shader/camera3d.vert",
			"assets/shader/camera3d.frag"
		};

		// Loaded host memory assets for this object's possession.
		this->Asset = Engine->FileManager.open(AssetList);

		// Allocate GPU resources.
		this->Framechain = std::dynamic_pointer_cast<framechain>(std::make_shared<geometry_buffer>(aContext, aCamera3DCreator->Resolution, aCamera3DCreator->FrameRate, aCamera3DCreator->FrameCount));

		// Grab shaders from asset list, compile, and link.
		std::shared_ptr<gcl::shader> VertexShader = std::dynamic_pointer_cast<gcl::shader>(Asset[0]);
		std::shared_ptr<gcl::shader> PixelShader = std::dynamic_pointer_cast<gcl::shader>(Asset[1]);
		std::vector<std::shared_ptr<gcl::shader>> ShaderList = { VertexShader, PixelShader };
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
		Rasterizer->attach(3, this->Framechain->draw_frame()["OGB.Depth"], 		image::layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

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

		camera_uniform_data UniformData = camera_uniform_data(
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

		camera_uniform_data UniformData = camera_uniform_data(
			this->Position, 
			this->DirectionRight, 
			this->DirectionUp, 
			this->DirectionFront, 
			this->FOV, 
			this->Framechain->Resolution, 
			this->Near,
			this->Far
		);
		memcpy(this->CameraUniformBuffer->Ptr, &UniformData, sizeof(camera_uniform_data));
	}

	std::vector<std::vector<ecs::object::draw_call>> camera3d::default_renderer(object* aObject) {
		// Gather list of mesh instances throughout model hierarchy.
		std::vector<mesh::instance*> MeshInstance = aObject->Model->Hierarchy.gather_mesh_instances();

		std::vector<std::vector<draw_call>> Renderer(this->Framechain->Image.size(), std::vector<draw_call>(MeshInstance.size()));

		for (size_t i = 0; i < this->Framechain->Image.size(); i++) {
			for (size_t j = 0; j < MeshInstance.size(); j++) {
				// Get references for readability.
				VkResult Result = VK_SUCCESS;
				std::shared_ptr<mesh> Mesh = aObject->Model->Mesh[MeshInstance[j]->Index];
				std::shared_ptr<material> Material = aObject->Model->Material[MeshInstance[j]->MaterialIndex];

				std::vector<std::shared_ptr<image>> ImageOutputList = {
					this->Framechain->Image[i]["OGB.Color"],
					this->Framechain->Image[i]["OGB.Position"],
					this->Framechain->Image[i]["OGB.Normal"],
					this->Framechain->Image[i]["OGB.Depth"]
				};
				Renderer[i][j].Framebuffer = Context->create_framebuffer(this->Pipeline, ImageOutputList, this->Framechain->Resolution);
				Renderer[i][j].DescriptorArray = Context->create_descriptor_array(this->Pipeline);
				Renderer[i][j].DrawCommand = this->CommandPool->allocate();

				// Bind Object Uniform Buffers
				Renderer[i][j].DescriptorArray->bind(0, 0, 0, this->CameraUniformBuffer);			// Camera Position, Orientation, Projection
				Renderer[i][j].DescriptorArray->bind(0, 1, 0, aObject->UniformBuffer);				// Object Position, Orientation, Scale
				Renderer[i][j].DescriptorArray->bind(0, 2, 0, MeshInstance[j]->UniformBuffer); 		// Mesh Instance Transform
				Renderer[i][j].DescriptorArray->bind(0, 3, 0, Material->UniformBuffer); 			// Material Properties

				// Bind Material Textures.
				Renderer[i][j].DescriptorArray->bind(1, 0, 0, Material->Texture["Color"]);
				Renderer[i][j].DescriptorArray->bind(1, 1, 0, Material->Texture["Normal"]);
				Renderer[i][j].DescriptorArray->bind(1, 2, 0, Material->Texture["Height"]);
				Renderer[i][j].DescriptorArray->bind(1, 3, 0, Material->Texture["Emission"]);
				Renderer[i][j].DescriptorArray->bind(1, 4, 0, Material->Texture["Opacity"]);
				Renderer[i][j].DescriptorArray->bind(1, 5, 0, Material->Texture["AmbientOcclusion"]);
				Renderer[i][j].DescriptorArray->bind(1, 6, 0, Material->Texture["MetallicRoughness"]);

				Result = Context->begin(Renderer[i][j].DrawCommand);
				std::vector<std::shared_ptr<buffer>> VertexBuffer = { Mesh->VertexBuffer, MeshInstance[j]->VertexWeightBuffer };
				this->Pipeline->draw(Renderer[i][j].DrawCommand, Renderer[i][j].Framebuffer, VertexBuffer, Mesh->IndexBuffer, Renderer[i][j].DescriptorArray);
				Result = Context->end(Renderer[i][j].DrawCommand);
			}
		}
		
		return Renderer;
	}


}
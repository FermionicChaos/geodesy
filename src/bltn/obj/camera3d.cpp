#include <geodesy/bltn/obj/camera3d.h>

namespace geodesy::bltn::obj {

	using namespace geodesy::core;
	using namespace geodesy::core::gfx;
	using namespace geodesy::core::gcl;

	// Localized to camera3d.cpp
	namespace {

		// This constitutes the default renderer of the geodesy
		// engine. Since this is localized to object.cpp, for any
		// derived class of object, it can override and define its
		// own renderer.
		class default_renderer : public core::gfx::renderer {
		public:

			default_renderer(core::gcl::context* aContext, camera3d* aCamera3D, ecs::object* aObject);

		};

		default_renderer::default_renderer(core::gcl::context* aContext, camera3d* aCamera3D, ecs::object* aObject) : core::gfx::renderer(aContext, aCamera3D, aObject) {

			// DescriptorSet for Object, Camera3D
			// DescriptorSet for mesh instance
			// DescriptorSet for material

			// Using the existing GPU resources of both Camera3D & the generic Object,
			// generate the proper draw calls for the object.

			// Camera3D has a default render pass, framebuffers encapsulating the various image views
			// for the images used by the render target. Along with a default set of pipelines used
			// for each subpass of the render pass.

			// Object has a model, which consists of a node hierarchy, rigged mesh instances, and so on.
			// The meshes them selves, and their node meta data is stored GPU side for each object. 

			// describes how camera will render object.
			std::shared_ptr<model> Model = aObject->Model;
			std::vector<std::shared_ptr<mesh>> MeshList = Model->Mesh;
			std::vector<mesh::instance*> MeshInstanceList = Model->Hierarchy.gather_mesh_instances();

			// Generate Descriptor Sets for each pipeline.

			this->DrawCall = std::vector<std::vector<draw_call>>(aCamera3D->Framechain->Image.size());
			for (size_t FrameIndex = 0; FrameIndex < aCamera3D->Framechain->Image.size(); FrameIndex++) {
				// Allocate a draw call per mesh instance.
				this->DrawCall[FrameIndex] = std::vector<draw_call>(MeshInstanceList.size());
				for (size_t InstanceIndex = 0; InstanceIndex < MeshInstanceList.size(); InstanceIndex++) {
					// Extract Mesh from MeshList.
					mesh::instance* MeshInstance = MeshInstanceList[InstanceIndex];
					std::shared_ptr<mesh> Mesh = MeshList[MeshInstance->Index];
					std::shared_ptr<material> Material = Model->Material[MeshInstance->MaterialIndex];

					// Create Command Buffer for draw call.
					VkCommandBuffer Cmd = aContext->allocate_command_buffer(device::GRAPHICS_AND_COMPUTE);
					std::shared_ptr<descriptor::array> Uniform;

					// VkRect2D RenderArea = { { 0, 0 }, { aCamera3D->Resolution[0], aCamera3D->Resolution[1] } };
					aContext->begin(Cmd);
					// aCamera3D->Pipeline->begin(Cmd, aCamera3D->Frame[FrameIndex].Buffer, RenderArea);
					//Uniform = aCamera3D->Pipeline[0]->create_uniform_array();

					// Bind Object & Camera3D Uniform Buffers.
					// Bind Mesh Instance Transform Uniform Buffer.
					// Bind Material Uniform Buffers & Textures.


					/*
					// Vertex Shader Bindings
					Uniform.bind(0, 0, Mesh->UniformBuffer);
					Uniform.bind(0, 1, Object->UniformBuffer);
					Uniform.bind(0, 2, Camera3D->UniformBuffer);

					// Pixel Shader Bindings
					Uniform.bind(1, 0, Mesh->Texture[0]);
					Uniform.bind(1, 1, Mesh->Texture[1]);
					Uniform.bind(1, 2, Mesh->Texture[2]);
					Uniform.bind(1, 3, Mesh->Texture[3]);
					Uniform.bind(1, 4, Mesh->Texture[4]);
					Uniform.bind(1, 5, Mesh->Texture[5]);
					*/
					// Bind Uniform Buffers here.
					// Mesh->draw(Cmd, aCamera3D->Pipeline, Uniform);
					aCamera3D->Pipeline->end(Cmd);
					aContext->end(Cmd);

					this->DrawCall[FrameIndex][InstanceIndex].Command = Cmd;

				}
			}
		}

	}

	camera3d::geometry_buffer::geometry_buffer(std::shared_ptr<core::gcl::context> aContext, core::math::vec<uint, 3> aResolution, double aFrameRate, size_t aFrameCount) : framechain(aContext, aFrameRate, aFrameCount) {
		// New API design?
		image::create_info DepthCreateInfo;
		DepthCreateInfo.Layout		= image::layout::SHADER_READ_ONLY_OPTIMAL;
		DepthCreateInfo.Sample		= image::sample::COUNT_1;
		DepthCreateInfo.Tiling		= image::tiling::OPTIMAL;
		DepthCreateInfo.Memory		= device::memory::DEVICE_LOCAL;
		DepthCreateInfo.Usage		= image::usage::SAMPLED | image::usage::DEPTH_STENCIL_ATTACHMENT  | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		image::create_info ColorCreateInfo;
		ColorCreateInfo.Layout		= image::layout::SHADER_READ_ONLY_OPTIMAL;
		ColorCreateInfo.Sample		= image::sample::COUNT_1;
		ColorCreateInfo.Tiling		= image::tiling::OPTIMAL;
		ColorCreateInfo.Memory		= device::memory::DEVICE_LOCAL;
		ColorCreateInfo.Usage		= image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		for (std::size_t i = 0; i < this->Image.size(); i++) {
			// This is the finalized color output.
			this->Image[i]["Color"] 		= aContext->create_image(ColorCreateInfo, image::R32G32B32A32_SFLOAT, aResolution[0], aResolution[1]);
			// Opaque Geometry Buffer, used for Opaque and Transparent mesh instances.
			this->Image[i]["OGB.Color"] 	= aContext->create_image(ColorCreateInfo, image::R32G32B32A32_SFLOAT, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Position"] 	= aContext->create_image(ColorCreateInfo, image::R32G32B32A32_SFLOAT, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Normal"] 	= aContext->create_image(ColorCreateInfo, image::R32G32B32A32_SFLOAT, aResolution[0], aResolution[1]);
			this->Image[i]["OGB.Depth"] 	= aContext->create_image(DepthCreateInfo, image::D32_SFLOAT, aResolution[0], aResolution[1]);
			this->Image[i]["FinalColor"] 	= aContext->create_image(ColorCreateInfo, image::R32G32B32A32_SFLOAT, aResolution[0], aResolution[1]);
		}
	}

	camera3d::camera3d(
		std::shared_ptr<core::gcl::context> 	aContext, 
		ecs::stage* 							aStage, 
		std::string 							aName, 
		core::math::vec<uint, 3> 				aFrameResolution, 
		double 									aFrameRate, 
		uint32_t 								aFrameCount
	) : ecs::subject(aContext, aStage, aName, aFrameResolution, aFrameRate, aFrameCount, 4) 
	{
		VkResult Result = VK_SUCCESS;
		engine* Engine = aContext->Device->Engine;

		// Create Geometry Buffer Images
		this->Framechain = std::dynamic_pointer_cast<core::gcl::framechain>(std::make_shared<geometry_buffer>(aContext, aFrameResolution, aFrameRate, aFrameCount));

		// Open Shaders.
		std::string VertexShaderPath = "assets/shader/camera3d.vert";
		std::string PixelShaderPath = "assets/shader/camera3d.frag";

		std::shared_ptr<gcl::shader> VertexShader = std::dynamic_pointer_cast<gcl::shader>(Engine->FileManager.open(VertexShaderPath));
		std::shared_ptr<gcl::shader> PixelShader = std::dynamic_pointer_cast<gcl::shader>(Engine->FileManager.open(PixelShaderPath));

		// Create Graphics Pipeline for rendering (REQUIRES RENDER PASS)
		{
			std::vector<std::shared_ptr<gcl::shader>> ShaderList = { VertexShader, PixelShader };
			pipeline::rasterizer Rasterizer(ShaderList, this->Framechain->Resolution, VK_FORMAT_D32_SFLOAT);

			// This code specifies how the vextex data is to be interpreted from the bound vertex buffers.
			Rasterizer.bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 0, offsetof(gfx::mesh::vertex, Position));
			Rasterizer.bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 1, offsetof(gfx::mesh::vertex, Normal));
			Rasterizer.bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 2, offsetof(gfx::mesh::vertex, Tangent));
			Rasterizer.bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 3, offsetof(gfx::mesh::vertex, Bitangent));
			Rasterizer.bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 4, offsetof(gfx::mesh::vertex, TextureCoordinate));
			Rasterizer.bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 5, offsetof(gfx::mesh::vertex, Color));
			Rasterizer.bind(VK_VERTEX_INPUT_RATE_VERTEX, 1, sizeof(gfx::mesh::vertex::weight), 6, offsetof(gfx::mesh::vertex::weight, BoneID));
			Rasterizer.bind(VK_VERTEX_INPUT_RATE_VERTEX, 1, sizeof(gfx::mesh::vertex::weight), 7, offsetof(gfx::mesh::vertex::weight, BoneWeight));

			// TODO: Fix Later, don't let VkAttachmentDescription be determined by shader parser.
			// Rasterizer.attach(0, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			// Rasterizer.attach(1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			// Rasterizer.attach(2, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			// Rasterizer.attach(3, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			// Rasterizer.attach(4, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // Depth Buffer

			// This code specifies how

			// How to intepret vertex data in rasterization.
			Rasterizer.InputAssembly.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			Rasterizer.InputAssembly.primitiveRestartEnable		= false;

			// Rasterizer Info
			Rasterizer.Rasterizer.rasterizerDiscardEnable		= VK_FALSE;
			Rasterizer.Rasterizer.polygonMode					= VK_POLYGON_MODE_FILL;
			Rasterizer.Rasterizer.cullMode						= VK_CULL_MODE_BACK_BIT;
			Rasterizer.Rasterizer.frontFace						= VK_FRONT_FACE_COUNTER_CLOCKWISE;

			// Copy Paste
			Rasterizer.Multisample.rasterizationSamples			= VK_SAMPLE_COUNT_1_BIT;

			// Oncoming Depth Value [OPERATOR] Depth Value In Buffer
			// Needed for 3D graphics.
			Rasterizer.DepthStencil.depthTestEnable				= VK_TRUE;
			Rasterizer.DepthStencil.depthWriteEnable			= VK_TRUE;
			Rasterizer.DepthStencil.depthCompareOp				= VK_COMPARE_OP_GREATER; // Camera, +z is closer.
			Rasterizer.DepthStencil.minDepthBounds				= -1.0f;
			Rasterizer.DepthStencil.maxDepthBounds				= +1.0f;

			// TODO: Changed rasterizer info into shared pointer for lifetime preservation, update later.
			//this->Pipeline = std::make_shared<gcl::pipeline>(aContext, Rasterizer);
		}


	}

	camera3d::~camera3d() {

	}

	std::shared_ptr<core::gfx::renderer> camera3d::make_default_renderer(object* aObject) {
		return std::make_shared<default_renderer>(this->Context.get(), this, aObject);
	}


}
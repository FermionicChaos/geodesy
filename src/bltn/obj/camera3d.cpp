#include <geodesy/bltn/obj/camera3d.h>

#include <iostream>

namespace geodesy::bltn::obj {

	using namespace geodesy::core;
	using namespace geodesy::core::gfx;
	using namespace geodesy::core::gcl;

	camera3d::geometry_buffer::geometry_buffer(std::shared_ptr<core::gcl::context> aContext, core::math::vec<uint, 3> aResolution, double aFrameRate, size_t aFrameCount) : framechain(aContext, aFrameRate, aFrameCount) {
		// New API design?
		image::create_info DepthCreateInfo;
		DepthCreateInfo.Layout		= image::layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		DepthCreateInfo.Memory		= device::memory::DEVICE_LOCAL;
		DepthCreateInfo.Usage		= image::usage::SAMPLED | image::usage::DEPTH_STENCIL_ATTACHMENT  | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		image::create_info ColorCreateInfo;
		ColorCreateInfo.Layout		= image::layout::SHADER_READ_ONLY_OPTIMAL;
		ColorCreateInfo.Memory		= device::memory::DEVICE_LOCAL;
		ColorCreateInfo.Usage		= image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		image::format ColorFormat = image::format::B8G8R8A8_UNORM; //image::format::R32G32B32A32_SFLOAT;
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
	}

	camera3d::camera3d(
		std::shared_ptr<core::gcl::context> 	aContext, 
		ecs::stage* 							aStage, 
		std::string 							aName, 
		core::math::vec<uint, 3> 				aFrameResolution, 
		double 									aFrameRate, 
		uint32_t 								aFrameCount
	) : ecs::subject(aContext, aStage, aName, aFrameResolution, aFrameRate, aFrameCount, 4, { 0.0f, -10.0f, 0.0f }, { 90.0f, 90.0f }) 
	{
		VkResult Result = VK_SUCCESS;
		engine* Engine = aContext->Device->Engine;

		// List of assets Camera3D will load into memory.
		std::vector<std::string> AssetList = {
			"assets/shader/camera3d.vert",
			"assets/shader/camera3d.frag"
		};

		// Loaded host memory assets for this object's possession.
		this->Asset = Engine->FileManager.open(AssetList);

		// Allocate GPU resources.
		this->Framechain = std::dynamic_pointer_cast<core::gcl::framechain>(std::make_shared<geometry_buffer>(aContext, aFrameResolution, aFrameRate, aFrameCount));

		// Grab shaders from asset list, compile, and link.
		std::shared_ptr<gcl::shader> VertexShader = std::dynamic_pointer_cast<gcl::shader>(Asset[0]);
		std::shared_ptr<gcl::shader> PixelShader = std::dynamic_pointer_cast<gcl::shader>(Asset[1]);
		std::vector<std::shared_ptr<gcl::shader>> ShaderList = { VertexShader, PixelShader };
		std::shared_ptr<pipeline::rasterizer> Rasterizer = std::make_shared<pipeline::rasterizer>(ShaderList, aFrameResolution, VK_FORMAT_D32_SFLOAT);

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
		Rasterizer->attach(0, this->draw_frame()["OGB.Color"], 		image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(1, this->draw_frame()["OGB.Position"], 	image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(2, this->draw_frame()["OGB.Normal"], 	image::layout::SHADER_READ_ONLY_OPTIMAL);
		Rasterizer->attach(3, this->draw_frame()["OGB.Depth"], 		image::layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		// How to intepret vertex data in rasterization.
		Rasterizer->InputAssembly.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		Rasterizer->InputAssembly.primitiveRestartEnable	= false;

		// Rasterizer Info
		Rasterizer->Rasterizer.rasterizerDiscardEnable		= VK_FALSE;
		Rasterizer->Rasterizer.polygonMode					= VK_POLYGON_MODE_FILL;
		Rasterizer->Rasterizer.cullMode						= VK_CULL_MODE_NONE;
		Rasterizer->Rasterizer.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// Copy Paste
		Rasterizer->Multisample.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;

		// Oncoming Depth Value [OPERATOR] Depth Value In Buffer
		// Needed for 3D graphics.
		Rasterizer->DepthStencil.depthTestEnable			= VK_TRUE;
		Rasterizer->DepthStencil.depthWriteEnable			= VK_TRUE;
		Rasterizer->DepthStencil.depthCompareOp				= VK_COMPARE_OP_GREATER; // Camera, +z is closer.
		// Rasterizer->DepthStencil.minDepthBounds				= -1.0f;
		// Rasterizer->DepthStencil.maxDepthBounds				= +1.0f;

		// Create render pipeline for camera3d.
		this->Pipeline = Context->create_pipeline(Rasterizer);

		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		camera_uniform_data UniformData;
		float AspectRatio = (float)aFrameResolution[1] / (float)aFrameResolution[0];
		UniformData.Position 		= this->Position;
		UniformData.Rotation 		= {
			DirectionRight[0], 		DirectionRight[1], 		DirectionRight[2], 		0.0f,
			-DirectionUp[0], 		-DirectionUp[1], 		-DirectionUp[2], 		0.0f,
			DirectionFront[0], 		DirectionFront[1], 		DirectionFront[2], 		0.0f,
			0.0f, 					0.0f, 					0.0f, 					1.0f
		};
		UniformData.Projection 		= math::perspective(math::radians(90.0f), AspectRatio, 0.1f, 100.0f);
		std::cout << UniformData.Projection << std::endl;

		this->CameraUniformBuffer = Context->create_buffer(UBCI, sizeof(UniformData), &UniformData);
		this->CameraUniformBuffer->map_memory(0, sizeof(UniformData));
	}

	camera3d::~camera3d() {

	}

	std::vector<std::vector<core::gfx::draw_call>> camera3d::default_renderer(object* aObject) {
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
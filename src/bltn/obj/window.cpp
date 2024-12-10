#include <geodesy/bltn/obj/window.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gcl;
	using namespace gfx;

	namespace {

		struct window_uniform_data {
			alignas(8) math::vec<float, 2> Size;
			alignas(16) math::vec<uint, 3> Resolution;
		};

	}

	window::property::property() {
		Resizable		= true;
		Decorated		= true;
		UserFocused		= true;
		AutoMinimize	= true;
		Floating		= false;
		Maximized		= false;
		Minimized		= false;
		Visible			= true;
		ScaleToMonitor	= false;
		CenterCursor	= true;
		FocusOnShow		= true;
		Fullscreen		= false;
		Hovered			= true;
	}

	window::window(
		std::shared_ptr<core::gcl::context> aContext, 
		ecs::stage* aStage, 
		std::string aName, 
		core::gcl::image::format aFormat,
		core::math::vec<uint, 3> aFrameResolution, 
		double aFrameRate, 
		uint32_t aFrameCount, 
		uint32_t aAttachmentCount
	) : ecs::subject(
		aContext, 
		aStage, 
		aName, 
		aFrameResolution, 
		aFrameRate, 
		aFrameCount, 
		aAttachmentCount
	) {
		// The default renderer of window is that it takes an image and samples it.
		// Uses only a quad for basic shaping features, and window parameters.
		///*
		engine* Engine = aContext->Device->Engine;
		std::vector<std::string> AssetPath = {
			"assets/models/quad.obj",
			"assets/shader/window.vert",
			"assets/shader/window.frag",
		};

		// Load assets into memory.
		this->Asset = Engine->FileManager.open(AssetPath);

		image::create_info TextureCreateInfo;
		TextureCreateInfo.Layout = image::layout::SHADER_READ_ONLY_OPTIMAL;
		TextureCreateInfo.Memory = device::memory::DEVICE_LOCAL;
		TextureCreateInfo.Usage	 = image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		// Convert Assets to their respective types after loading into memory.
		std::shared_ptr<model> HostModel = std::dynamic_pointer_cast<model>(Asset[0]);
		std::shared_ptr<shader> VertexShader = std::dynamic_pointer_cast<shader>(Asset[1]);
		std::shared_ptr<shader> PixelShader = std::dynamic_pointer_cast<shader>(Asset[2]);

		// Convert Host model into device model.
		this->Model = std::make_shared<model>(aContext, HostModel, TextureCreateInfo);
		std::vector<std::shared_ptr<shader>> ShaderList = { VertexShader, PixelShader };
		std::shared_ptr<pipeline::rasterizer> Rasterizer = std::make_shared<pipeline::rasterizer>(ShaderList, aFrameResolution);

		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 0, offsetof(gfx::mesh::vertex, Position));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 1, offsetof(gfx::mesh::vertex, Normal));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 2, offsetof(gfx::mesh::vertex, Tangent));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 3, offsetof(gfx::mesh::vertex, Bitangent));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 4, offsetof(gfx::mesh::vertex, TextureCoordinate));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 5, offsetof(gfx::mesh::vertex, Color));

		// TODO: Change to shader read only optimal later?
		Rasterizer->attach(0, aFormat, image::sample::COUNT_1, image::layout::PRESENT_SRC_KHR);

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

		this->Pipeline = Context->create_pipeline(Rasterizer);

		// Allocate Uniform Buffer Info.
		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
		window_uniform_data WindowUniformData;
		WindowUniformData.Size = { 0.0f, 0.0f };
		WindowUniformData.Resolution = aFrameResolution;
		this->WindowUniformBuffer = aContext->create_buffer(UBCI, sizeof(window_uniform_data), &WindowUniformData);
		this->WindowUniformBuffer->map_memory(0, sizeof(window_uniform_data));
	}

	void window::initialize_pipeline() {

	}

	std::vector<std::vector<core::gfx::draw_call>> window::default_renderer(ecs::object* aObject) {
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
					this->Framechain->Image[i]["Color"]
				};
				Renderer[i][j].Framebuffer = Context->create_framebuffer(this->Pipeline, ImageOutputList, this->Framechain->Resolution);
				Renderer[i][j].DescriptorArray = Context->create_descriptor_array(this->Pipeline);
				Renderer[i][j].DrawCommand = this->CommandPool->allocate();

				// Bind Object Uniform Buffers
				Renderer[i][j].DescriptorArray->bind(0, 0, 0, this->WindowUniformBuffer);			// Window Size, etc
				Renderer[i][j].DescriptorArray->bind(0, 1, 0, aObject->UniformBuffer);				// Object Position, Orientation, Scale
				Renderer[i][j].DescriptorArray->bind(0, 2, 0, Material->UniformBuffer); 			// Material Properties

				// Bind Material Textures.
				Renderer[i][j].DescriptorArray->bind(1, 0, 0, Material->Texture["Color"]);

				Result = Context->begin(Renderer[i][j].DrawCommand);
				std::vector<std::shared_ptr<buffer>> VertexBuffer = { Mesh->VertexBuffer, MeshInstance[j]->VertexWeightBuffer };
				this->Pipeline->draw(Renderer[i][j].DrawCommand, Renderer[i][j].Framebuffer, VertexBuffer, Mesh->IndexBuffer, Renderer[i][j].DescriptorArray);
				Result = Context->end(Renderer[i][j].DrawCommand);
			}
		}
		
		return Renderer;
	}
	
}

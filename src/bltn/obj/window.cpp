#include <geodesy/bltn/obj/window.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gpu;
	using namespace gfx;

	namespace {

		struct window_uniform_data {
			alignas(8) math::vec<float, 2> Size;
			alignas(16) math::vec<uint, 3> Resolution;
			window_uniform_data(
				math::vec<float, 2> aSize,
				math::vec<uint, 3> aResolution
			);
		};

		window_uniform_data::window_uniform_data(
			math::vec<float, 2> aSize,
			math::vec<uint, 3> aResolution
		) {
			this->Size = aSize;
			this->Resolution = aResolution;
		}

	}

	window::window_draw_call::window_draw_call(
		object* 								aObject, 
		core::gfx::mesh::instance* 				aMeshInstance,
		window* 								aWindow,
		size_t 									aFrameIndex
	) {
		// Get references for readability.
		VkResult Result = VK_SUCCESS;
		std::shared_ptr<gpu::context> Context = aObject->Context;
		std::shared_ptr<mesh> Mesh = aObject->Model->Mesh[aMeshInstance->MeshIndex];
		std::shared_ptr<material> Material = aObject->Model->Material[aMeshInstance->MaterialIndex];

		// Get image outputs and vertex inputs.
		std::vector<std::shared_ptr<image>> ImageOutputList = {
			aWindow->Framechain->Image[aFrameIndex]["Color"]
		};
		std::vector<std::shared_ptr<buffer>> VertexBuffer = { Mesh->VertexBuffer, aMeshInstance->VertexWeightBuffer };

		// Allocate GPU resources to interface with pipeline.
		Framebuffer 		= Context->create_framebuffer(aWindow->Pipeline, ImageOutputList, aWindow->Framechain->Resolution);
		DescriptorArray 	= Context->create_descriptor_array(aWindow->Pipeline);
		DrawCommand 		= aWindow->CommandPool->allocate();

		// Bind Object Uniform Buffers
		DescriptorArray->bind(0, 0, 0, aWindow->WindowUniformBuffer);		// Window Size, etc
		DescriptorArray->bind(0, 1, 0, aObject->UniformBuffer);				// Object Position, Orientation, Scale
		DescriptorArray->bind(0, 2, 0, Material->UniformBuffer); 			// Material Properties

		// Bind Material Textures.
		DescriptorArray->bind(1, 0, 0, Material->Texture["Color"]);

		Result = Context->begin(DrawCommand);
		aWindow->Pipeline->draw(DrawCommand, Framebuffer, VertexBuffer, Mesh->IndexBuffer, DescriptorArray);
		Result = Context->end(DrawCommand);
	}

	window::window_renderer::window_renderer(runtime::object* aObject, window* aWindow) : runtime::object::renderer(aObject, aWindow) {
		// Gather list of mesh instances throughout model hierarchy.
		std::vector<mesh::instance*> MeshInstance = aObject->gather_instances();

		std::vector<std::vector<draw_call>> Renderer(aWindow->Framechain->Image.size(), std::vector<draw_call>(MeshInstance.size()));

		// Generate draw calls standard per frame and mesh instance.
		for (size_t i = 0; i < aWindow->Framechain->Image.size(); i++) {
			for (size_t j = 0; j < MeshInstance.size(); j++) {
				DrawCallList[i][j] = geodesy::make<window_draw_call>(aObject, MeshInstance[j], aWindow, i);
			}
		}
	}

	window::creator::creator() {
		ModelPath 		= "assets/models/quad.obj";
		PixelFormat		= image::format::B8G8R8A8_UNORM;
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

	window::window(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aWindowCreator) : runtime::subject(aContext, aStage, aWindowCreator) {
		// The default renderer of window is that it takes an image and samples it.
		// Uses only a quad for basic shaping features, and window parameters. Mostly used for 2D graphics.
		// Load in shaders for rendering.
		std::vector<std::string> AssetPath = {
			"assets/shader/window/window.vert",
			"assets/shader/window/window.frag",
		};

		// Open Shader Files.
		std::vector<std::shared_ptr<core::io::file>> NewAsset = Engine->FileManager.open(AssetPath);

		// Insert Shader Files into Asset List.
		this->Asset.insert(this->Asset.end(), NewAsset.begin(), NewAsset.end());	

		// Convert Assets to their respective types after loading into memory.
		std::shared_ptr<shader> VertexShader = std::dynamic_pointer_cast<shader>(NewAsset[0]);
		std::shared_ptr<shader> PixelShader = std::dynamic_pointer_cast<shader>(NewAsset[1]);

		// Convert Host model into device model.
		std::vector<std::shared_ptr<shader>> ShaderList = { VertexShader, PixelShader };
		std::shared_ptr<pipeline::rasterizer> Rasterizer = std::make_shared<pipeline::rasterizer>(ShaderList, aWindowCreator->Resolution);

		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 0, offsetof(gfx::mesh::vertex, Position));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 1, offsetof(gfx::mesh::vertex, Normal));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 2, offsetof(gfx::mesh::vertex, Tangent));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 3, offsetof(gfx::mesh::vertex, Bitangent));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 4, offsetof(gfx::mesh::vertex, TextureCoordinate));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 5, offsetof(gfx::mesh::vertex, Color));

		// Set output format.
		Rasterizer->attach(0, aWindowCreator->PixelFormat, image::sample::COUNT_1, image::layout::SHADER_READ_ONLY_OPTIMAL);

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
		
		window_uniform_data WindowUniformData = window_uniform_data({ 1.0f, 1.0f }, aWindowCreator->Resolution);
		this->WindowUniformBuffer = aContext->create_buffer(UBCI, sizeof(window_uniform_data), &WindowUniformData);
		this->WindowUniformBuffer->map_memory(0, sizeof(window_uniform_data));
	}

	std::shared_ptr<runtime::object::renderer> window::default_renderer(runtime::object* aObject) {
		return std::dynamic_pointer_cast<runtime::object::renderer>(std::make_shared<window_renderer>(aObject, this));
	}
	
}

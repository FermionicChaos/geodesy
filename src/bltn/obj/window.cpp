#include <geodesy/bltn/obj/window.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gpu;
	using namespace gfx;

	window::window_draw_call::window_draw_call(
		window* aWindow,
		size_t 	aFrameIndex,
		object* aObject, 
		size_t 	aMeshInstanceIndex
	) {
		// Get references for readability.
		VkResult Result = VK_SUCCESS;
		auto Context = aObject->Context;
		auto MeshInstance = aObject->TotalMeshInstance[aMeshInstanceIndex];
		auto Mesh = aObject->Model->Mesh[MeshInstance->MeshIndex];
		auto Material = aObject->Model->Material[MeshInstance->MaterialIndex];
		auto Node = MeshInstance->Parent;

		// Get image outputs and vertex inputs.
		std::vector<std::shared_ptr<image>> ImageOutputList = {
			aWindow->Framechain->Image[aFrameIndex]["Color"]
		};
		std::vector<std::shared_ptr<buffer>> VertexBuffer = { Mesh->VertexBuffer, MeshInstance->VertexWeightBuffer };

		// Allocate GPU resources to interface with pipeline.
		Framebuffer 		= Context->create_framebuffer(aWindow->Pipeline[0], ImageOutputList, aWindow->Framechain->Resolution);
		DescriptorArray 	= Context->create_descriptor_array(aWindow->Pipeline[0]);
		DrawCommand 		= aWindow->CommandPool->allocate();

		// Bind Object Uniform Buffers
		DescriptorArray->bind(0, 0, 0, aWindow->SubjectUniformBuffer);			// Camera Position, Orientation, Projection
		DescriptorArray->bind(0, 1, 0, MeshInstance->UniformBuffer); 			// Mesh Instance Transform
		DescriptorArray->bind(0, 2, 0, Material->UniformBuffer); 				// Material Properties

		// Bind Material Textures.
		DescriptorArray->bind(1, 0, 0, Material->Texture["Color"]);
		DescriptorArray->bind(1, 1, 0, Material->Texture["Opacity"]);

		Result = Context->begin(DrawCommand);
		aWindow->Pipeline[0]->draw(DrawCommand, Framebuffer, VertexBuffer, Mesh->IndexBuffer, DescriptorArray);
		Result = Context->end(DrawCommand);
	}

	window::window_renderer::window_renderer(runtime::object* aObject, window* aWindow) : runtime::object::renderer(aWindow, aObject) {
		// Gather list of mesh instances throughout model hierarchy.
		std::vector<mesh::instance*> MeshInstance = aObject->gather_instances();

		std::vector<std::vector<draw_call>> Renderer(aWindow->Framechain->Image.size(), std::vector<draw_call>(MeshInstance.size()));

		// Generate draw calls standard per frame and mesh instance.
		for (size_t i = 0; i < aWindow->Framechain->Image.size(); i++) {
			for (size_t j = 0; j < MeshInstance.size(); j++) {
				DrawCallList[i][j] = geodesy::make<window_draw_call>(aWindow, i, aObject, j);
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
			"assets/shader/standard.vert",
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

		// ----- Set Pipeline Options ----- //
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

		// Define Vertex Input Bindings
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 0, offsetof(gfx::mesh::vertex, Position));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 1, offsetof(gfx::mesh::vertex, Normal));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 2, offsetof(gfx::mesh::vertex, Tangent));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 3, offsetof(gfx::mesh::vertex, Bitangent));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 4, offsetof(gfx::mesh::vertex, TextureCoordinate));
		Rasterizer->bind(0, sizeof(gfx::mesh::vertex), 5, offsetof(gfx::mesh::vertex, Color));

		// Define Output Attachments
		Rasterizer->attach(0, aWindowCreator->PixelFormat);

		this->Pipeline = std::vector<std::shared_ptr<core::gpu::pipeline>>(1);
		this->Pipeline[0] = Context->create_pipeline(Rasterizer);

		// Allocate Uniform Buffer Info.
		subject::uniform_data UBO = subject::uniform_data(
			this->Position, 
			{ this->Theta, this->Phi },
			this->Scale,
			-1.0f,
			1.0f
		);

		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		this->SubjectUniformBuffer = Context->create_buffer(UBCI, sizeof(subject::uniform_data), &UBO);
		this->SubjectUniformBuffer->map_memory(0, sizeof(subject::uniform_data));
	}

	std::shared_ptr<runtime::object::renderer> window::default_renderer(runtime::object* aObject) {
		return std::dynamic_pointer_cast<runtime::object::renderer>(std::make_shared<window_renderer>(aObject, this));
	}
	
}

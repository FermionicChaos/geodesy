#include <geodesy/bltn/obj/window.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gcl;
	using namespace gfx;

		// Localized to camera3d.cpp
	namespace {

		// This constitutes the default renderer of the geodesy
		// engine. Since this is localized to object.cpp, for any
		// derived class of object, it can override and define its
		// own renderer.
		class default_renderer : public core::gfx::renderer {
		public:

			default_renderer(core::gcl::context* aContext, window* aWindow, ecs::object* aObject);

		};

		default_renderer::default_renderer(core::gcl::context* aContext, window* aWindow, ecs::object* aObject) : core::gfx::renderer(aContext, aWindow, aObject) {

			
		}

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

	window::window(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, std::string aName, core::math::vec<uint, 3> aFrameResolution, double aFrameRate, uint32_t aFrameCount, uint32_t aAttachmentCount) :
	ecs::subject(aContext, aStage, aName, aFrameResolution, aFrameRate, aFrameCount, aAttachmentCount) {
		// The default renderer of window is that it takes an image and samples it.
		// Uses only a quad for basic shaping features, and window parameters.
		/*
		engine* Engine = aContext->Device->Engine;
		std::vector<std::string> AssetPath = {
			"assets/models/quad.obj",
			"assets/shader/window.vert",
			"assets/shader/window.frag",
		};

		// Load assets into memory.
		std::vector<std::shared_ptr<io::file>> Asset = Engine->FileManager.open(AssetPath);

		image::create_info TextureCreateInfo;
		TextureCreateInfo.Memory = device::memory::DEVICE_LOCAL;
		TextureCreateInfo.Usage	 = image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;
		TextureCreateInfo.Layout = image::layout::SHADER_READ_ONLY_OPTIMAL;
		TextureCreateInfo.Sample = image::sample::COUNT_1;
		TextureCreateInfo.Tiling = image::tiling::OPTIMAL;

		// Convert Assets to their respective types after loading into memory.
		std::shared_ptr<model> HostModel = std::dynamic_pointer_cast<model>(Asset[0]);
		std::shared_ptr<shader> VertexShader = std::dynamic_pointer_cast<shader>(Asset[1]);
		std::shared_ptr<shader> PixelShader = std::dynamic_pointer_cast<shader>(Asset[2]);

		// Convert Host model into device model.
		this->Model = std::make_shared<model>(aContext, HostModel, TextureCreateInfo);
		std::vector<std::shared_ptr<shader>> ShaderList = { VertexShader, PixelShader };
		this->Rasterizer = std::make_shared<pipeline::rasterizer>(ShaderList, aFrameResolution);

		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 0, offsetof(gfx::mesh::vertex, Position));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 1, offsetof(gfx::mesh::vertex, Normal));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 2, offsetof(gfx::mesh::vertex, Tangent));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 3, offsetof(gfx::mesh::vertex, Bitangent));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 4, offsetof(gfx::mesh::vertex, TextureCoordinate));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 5, offsetof(gfx::mesh::vertex, Color));

		// ! Framechain not initialized. (Maybe initialize later?)
		// Rasterizer->attach(0, this->Framechain->Image[0]["Color"], image::layout::PRESENT_SRC_KHR);

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
		*/


	}

	std::vector<std::vector<core::gfx::draw_call>> window::default_renderer(ecs::object* aObject) {
		std::vector<std::vector<core::gfx::draw_call>> Renderer;
		return Renderer;
	}
	
}

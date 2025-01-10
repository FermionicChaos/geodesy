#include <geodesy/engine.h>

#include <geodesy/bltn.h>

bool ThirdPartyLibrariesInitialized = false;
int EngineInstanceCount = 0;

namespace geodesy {

	using namespace core;
	using namespace bltn::obj;
	//using namespace bltn::stg;
	//using namespace bltn::app;

	using namespace util;
	using namespace gcl;
	using namespace lgc;
	using namespace gfx;

	static bool Initialized = false;

	bool engine::initialize() {
		// This is to prevent loading twice. Initializes all third party libraries for the lifetime of the program.
		if (Initialized) {
			return true;
		}

		// Initialize Third Party Libraries.
		bool Checker = true;

		// Initialize Shader Reflection API glslang.
		Checker &= shader::initialize();

		// Initialize Font Loading Library FreeType.
		// Checker &= font::initialize();

		// Initialize Image Loading Library FreeImage.
		// Checker &= image::initialize();

		// Initialize Model Loading Library Assimp.
		Checker &= model::initialize();

		// Initialize Audio Loading Library PortAudio.
		// Checker &= audio::initialize();

		// Initialize Window System Integration GLFW.
		Checker &= system_window::initialize();

		// Set Initialized to true.
		Initialized = Checker;

		// If not initialized, terminate all third party libraries.
		if (!Initialized) {
			engine::terminate();
		}

		return Initialized;
	}

	void engine::terminate() {
		if (Initialized) {

			// Terminate Window System Integration GLFW.
			system_window::terminate();

			// Terminate Audio Loading Library PortAudio.
			// audio::terminate();

			// Terminate Model Loading Library Assimp.
			model::terminate();

			// Terminate Image Loading Library FreeImage.
			// image::terminate();

			// Terminate Font Loading Library FreeType.
			// font::terminate();

			// Terminate Shader Reflection API glslang.
			shader::terminate();

			// Set Initialized to false.
			Initialized = false;
		}
	}
	
	engine::engine() {
		this->Handle = VK_NULL_HANDLE;
		this->PrimaryDisplay = nullptr;
		this->PrimaryDevice = nullptr;
	}

	engine::engine(std::vector<const char*> aCommandLineArgumentList, std::vector<const char*> aLayerList, std::vector<const char*> aExtensionList) : engine() {
		VkResult Result = VK_SUCCESS;
		this->Name			= "Geodesy Engine";
		this->Version		= math::vec<uint, 3>(GEODESY_ENGINE_VERSION_MAJOR, GEODESY_ENGINE_VERSION_MINOR, GEODESY_ENGINE_VERSION_PATCH);

		// Initialize Vulkan Graphics & Computation API.
		{
			VkApplicationInfo AppInfo{};
			VkInstanceCreateInfo CreateInfo{};
			// Load Default Extensions
			std::vector<const char*> Layer;
			std::vector<const char*> Extension;

			// Add Validation Layers
			Layer.insert(Layer.end(), aLayerList.begin(), aLayerList.end());

			// Add WSI extensions
			Extension.insert(Extension.end(), aExtensionList.begin(), aExtensionList.end());
			
			// TODO: Figure out how to funnel app meta data into here.
			AppInfo.sType								= VK_STRUCTURE_TYPE_APPLICATION_INFO;
			AppInfo.pNext								= NULL;
			AppInfo.pApplicationName					= "";
			AppInfo.applicationVersion					= VK_MAKE_VERSION(0, 0, 0);
			AppInfo.pEngineName							= this->Name.c_str();
			AppInfo.engineVersion						= VK_MAKE_VERSION(Version[0], Version[1], Version[2]);
			AppInfo.apiVersion							= VK_API_VERSION_1_3;

			CreateInfo.sType							= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			CreateInfo.pNext							= NULL;
			CreateInfo.flags							= 0;
			CreateInfo.pApplicationInfo					= &AppInfo;
			CreateInfo.enabledLayerCount				= Layer.size();
			CreateInfo.ppEnabledLayerNames				= Layer.data();
			CreateInfo.enabledExtensionCount			= Extension.size();
			CreateInfo.ppEnabledExtensionNames			= Extension.data();

			Result = vkCreateInstance(&CreateInfo, NULL, &this->Handle);
			if (Result == VK_SUCCESS) {
				Logger << log::message(log::VULKAN, log::INFO, log::SUCCESS, "Vulkan Graphics & Computation Initialization Successful!");
			}
			else {
				Logger << log::message(log::VULKAN, Result, "Vulkan Graphics & Computation Initialization Failed!");
				throw Logger;
			}
		}

		// Query system for existing GPGPU capable devices.
		gcl::device::get_system_devices(this);
		if (this->Device.size() == 0) {
			Logger << log::message(log::GEODESY, log::ERROR, log::INITIALIZATION_FAILED, "Error: No Vulkan capable devices detected on system!");
			throw Logger;
		}

		// Query system for existing displays.
		system_display::get_system_displays(this);
		if (this->Display.size() == 0) {
			Logger << log::message(log::GEODESY, log::ERROR, log::INITIALIZATION_FAILED, "Error: No Displays detected on system!");
			throw Logger;
		}

	}

	engine::~engine() {

	}

	std::shared_ptr<core::gcl::context> engine::create_device_context(std::shared_ptr<core::gcl::device> aDevice, std::vector<uint> aOperationBitfieldList, std::vector<const char*> aLayerList, std::vector<const char*> aExtensionList) {
		std::shared_ptr<core::gcl::context> NewDeviceContext = std::make_shared<core::gcl::context>(aDevice, aOperationBitfieldList, aLayerList, aExtensionList);
		Context.insert(NewDeviceContext);
		return NewDeviceContext;
	}

	void engine::destroy_device_context(std::shared_ptr<core::gcl::context> aDeviceContext) {
		Context.erase(aDeviceContext);
		// Should I delete all contexts everywhere, or allow resoure deallocation until?
	}

	VkResult engine::wait_on_device_context(std::vector<std::shared_ptr<core::gcl::context>> aDeviceContextList) {
		VkResult Result = VK_SUCCESS;
		if (aDeviceContextList.size() == 0) {
			for (auto& Ctx : Context) {
				Result = vkDeviceWaitIdle(Ctx->Handle);
			}
		}
		else {
			for (auto& Ctx : aDeviceContextList) {
				Result = vkDeviceWaitIdle(Ctx->Handle);
			}
		}
		return Result;
	}

	void engine::run(ecs::app* aApp) {

		aApp->init();

	}

	VkResult engine::update_resources(ecs::app* aApp) {
		VkResult Result = VK_SUCCESS;
		std::map<std::shared_ptr<context>, core::gcl::submission_batch> UpdateOperations;

		aApp->Mutex.lock();

		UpdateOperations = aApp->update(aApp->TimeStep);

		aApp->Mutex.unlock();

		// --------------- Per Device Context work is done here --------------- //

		for (std::shared_ptr<context> Ctx : Context) {
			// Lock Context for execution.
			Ctx->Mutex.lock();

			// Wait for other inflight operations to finish.
			Result = Ctx->engine_wait({ device::operation::TRANSFER_AND_COMPUTE, device::operation::GRAPHICS_AND_COMPUTE });

			// Execute all transfer device operations.
			Result = Ctx->engine_execute(device::operation::TRANSFER_AND_COMPUTE, UpdateOperations[Ctx].SubmitInfo);

			// Unlock device context.
			Ctx->Mutex.unlock();
		}

		return Result;
	}

	VkResult engine::execute_render_operations(ecs::app* aApp) {
		VkResult Result = VK_SUCCESS;
		std::map<std::shared_ptr<context>, core::gcl::submission_batch> RenderInfo;

		aApp->Mutex.lock();

		RenderInfo = aApp->render();

		aApp->Mutex.unlock();

		// --------------- Per Device Context work is done here --------------- //

		for (std::shared_ptr<context> Ctx : Context) {
			// Lock Context for execution.
			Ctx->Mutex.lock();

			// Wait for other inflight operations to finish.
			Result = Ctx->engine_wait({ device::operation::TRANSFER_AND_COMPUTE, device::operation::GRAPHICS_AND_COMPUTE });

			// Execute all transfer device operations.
			Result = Ctx->engine_execute(device::operation::GRAPHICS_AND_COMPUTE, RenderInfo[Ctx].SubmitInfo);

			// Unlock device context.
			Ctx->Mutex.unlock();
		}

		return Result;
	}

}

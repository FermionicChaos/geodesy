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

	engine::engine() {
		this->Handle = VK_NULL_HANDLE;
		this->PrimaryDisplay = nullptr;
		this->PrimaryDevice = nullptr;
	}

	engine::engine(std::vector<const char*> aCommandLineArgumentList, std::vector<const char*> aLayerList, std::vector<const char*> aExtensionList) : engine() {
		VkResult Result = VK_SUCCESS;
		this->Name			= "Geodesy Engine";
		this->Version		= math::vec<uint, 3>(GEODESY_ENGINE_VERSION_MAJOR, GEODESY_ENGINE_VERSION_MINOR, GEODESY_ENGINE_VERSION_PATCH);

		// Initialize Third Party Libraries.
		{
			if (shader::initialize()) {
				Logger << log::message(log::GEODESY, log::INFO, log::SUCCESS, "Shader Reflection API Initializetion Successful!");
			}
			else {
				Logger << log::message(log::GEODESY, log::ERROR, log::INITIALIZATION_FAILED, "Shader Reflection API Initializetion Failed!");
				throw Logger;
			}

			// Initialize Model Loading Library
			if (model::initialize()) {
				Logger << log::message(log::GEODESY, log::INFO, log::SUCCESS, "Model Loading Library Initializetion Successful!");
			}
			else {
				Logger << log::message(log::GEODESY, log::ERROR, log::INITIALIZATION_FAILED, "Model Loading Library Initializetion Failed!");
				throw Logger;
			}

			// Window System Integration
			if (system_window::initialize()) {
				Logger << log::message(log::GEODESY, log::INFO, log::SUCCESS, "Window System Integration Initializetion Successful!");
			}
			else {
				Logger << log::message(log::GEODESY, log::ERROR, log::INITIALIZATION_FAILED, "Window System Integration Initializetion Failed!");
				throw Logger;
			}

		}

		// Initialize Vulkan Graphics & Computation API.
		{
			VkApplicationInfo AppInfo{};
			VkInstanceCreateInfo CreateInfo{};
			// Load Default Extensions
			std::vector<const char*> Layer = { 
				"VK_LAYER_KHRONOS_validation"
			};
			std::vector<const char*> Extension = system_window::engine_extensions();

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

	void engine::run(ecs::app* aApp) {

		aApp->init();

	}

	VkResult engine::update_resources(ecs::app* aApp) {
		VkResult Result = VK_SUCCESS;
		std::map<std::shared_ptr<context>, ecs::object::update_info> UpdateOperations;

		aApp->Mutex.lock();

		UpdateOperations = aApp->update(aApp->TimeStep);

		aApp->Mutex.unlock();

		// --------------- Per Device Context work is done here --------------- //

		for (std::shared_ptr<context> Ctx : Context) {
			// Lock Context for execution.
			Ctx->Mutex.lock();

			// Wait for other inflight operations to finish.
			Result = Ctx->engine_wait({ device::operation::TRANSFER, device::operation::COMPUTE, device::operation::GRAPHICS_AND_COMPUTE });

			// Execute all transfer device operations.
			Result = Ctx->engine_execute(device::operation::TRANSFER, UpdateOperations[Ctx].TransferOperations);

			// Execute all compute device operations.
			Result = Ctx->engine_execute(device::operation::COMPUTE, UpdateOperations[Ctx].ComputeOperations);

			// Unlock device context.
			Ctx->Mutex.unlock();
		}

		return Result;
	}

	VkResult engine::execute_render_operations(ecs::app* aApp) {
		VkResult Result = VK_SUCCESS;
		std::map<std::shared_ptr<context>, ecs::subject::render_info> RenderInfo;

		aApp->Mutex.lock();

		RenderInfo = aApp->render();

		aApp->Mutex.unlock();

		// --------------- Per Device Context work is done here --------------- //

		for (std::shared_ptr<context> Ctx : Context) {
			// Lock Context for execution.
			Ctx->Mutex.lock();

			// Wait for other inflight operations to finish.
			Result = Ctx->engine_wait({ device::operation::TRANSFER, device::operation::COMPUTE, device::operation::GRAPHICS_AND_COMPUTE });

			// Execute all transfer device operations.
			Result = Ctx->engine_execute(device::operation::GRAPHICS_AND_COMPUTE, RenderInfo[Ctx].SubmitInfo);

			// Unlock device context.
			Ctx->Mutex.unlock();

			// Execute all system window presentation operations.
			Result = Ctx->present(RenderInfo[Ctx].PresentInfo);
		}

		return Result;
	}

}

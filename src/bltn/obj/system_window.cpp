#include <geodesy/bltn/obj/system_window.h>

#include <GLFW/glfw3.h>

#include <iostream>

/* --------------- Platform Dependent Libraries --------------- */
//#if defined(_WIN32) || defined(_WIN64)
//#include <Windows.h>
//#elif defined(__APPLE__) || defined(MACOSX)
//
//#elif defined(__linux__)
//#include <unistd.h>
//#endif

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace util;
	using namespace gcl;

	// // Signals to update thread to create window handle
	// // Needed backend for system window creation
	// std::thread::id system_window::MainThreadID								;
	// std::atomic<bool> system_window::WindowCreated							= false;
	// std::atomic<bool> system_window::WindowCreationRequest					= false;
	// system_window::window_creation_arguments system_window::WindowArg		= system_window::window_creation_arguments();
	// std::atomic<GLFWwindow*> system_window::ReturnWindow					= NULL;
	// std::atomic<GLFWwindow*> system_window::DestroyWindow					= NULL;


	bool system_window::initialize() {
		return (glfwInit() == GLFW_TRUE);
	}

	void system_window::terminate() {
		glfwTerminate();
	}

	std::vector<const char*> system_window::engine_extensions() {
		std::vector<const char*> EngineExtension;
		uint32_t EC = 0;
		const char** EL = glfwGetRequiredInstanceExtensions(&EC);
		for (uint32_t i = 0; i < EC; i++) {
			EngineExtension.push_back(EL[i]);
		}
		return EngineExtension;
	}

	std::vector<const char*> system_window::context_extensions() {
		std::vector<const char*> ContextExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		return ContextExtension;
	}

	void system_window::check_present_support(core::gcl::device* aDevice) {
		VkResult Result = VK_SUCCESS;
		GLFWwindow* DummyWindow = NULL;
		VkSurfaceKHR DummySurface = VK_NULL_HANDLE;
		// Will check existing queue families for present support.

		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_RESIZABLE,			GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED,			GLFW_TRUE);
		glfwWindowHint(GLFW_FOCUSED,			GLFW_TRUE);
		glfwWindowHint(GLFW_AUTO_ICONIFY,		GLFW_TRUE);
		glfwWindowHint(GLFW_FLOATING,			GLFW_FALSE);
		glfwWindowHint(GLFW_MAXIMIZED,			GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE,			GLFW_FALSE);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR,	GLFW_FALSE);
		glfwWindowHint(GLFW_CENTER_CURSOR,		GLFW_TRUE);
		glfwWindowHint(GLFW_FOCUS_ON_SHOW,		GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API,			GLFW_NO_API);
		glfwWindowHint(GLFW_REFRESH_RATE,		GLFW_DONT_CARE);

		// Create dummy window for testing.
		DummyWindow = glfwCreateWindow(640, 480, "Dummy Window", NULL, NULL);
		if (DummyWindow == NULL) {
			const char* Description = NULL;
			int ErrorCode = glfwGetError(&Description);
			aDevice->Engine->Logger << log::message(log::GLFW, ErrorCode, Description);
			throw aDevice->Engine->Logger;
		}

		glfwDefaultWindowHints();

		// Create Dummy Surface for testing.
		Result = glfwCreateWindowSurface(aDevice->Engine->Handle, DummyWindow, NULL, &DummySurface);
		if (Result != VK_SUCCESS) {
			aDevice->Engine->Logger << log::message(log::VULKAN, Result, "Error: Failed to create Dummy Surface!");
			throw aDevice->Engine->Logger;
		}

		// Get present support.
		for (size_t i = 0; i < aDevice->QueueFamilyProperty.size(); i++) {
			VkBool32 PresentSupport = VK_FALSE;
			Result = vkGetPhysicalDeviceSurfaceSupportKHR(aDevice->Handle, i, DummySurface, &PresentSupport);
			if (PresentSupport == VK_TRUE) {
				aDevice->QueueFamilyProperty[i].OperationBitfield |= core::gcl::device::operation::PRESENT;
				aDevice->QueueFamilyProperty[i].OperationList.push_back(core::gcl::device::operation::PRESENT);
			}
		}
	}

	double system_window::get_time() {
		return glfwGetTime();
	}

	void system_window::set_time(double aTime) {
		glfwSetTime(aTime);
	}

	void system_window::wait(double aSeconds) {
		unsigned long long Nanoseconds = (unsigned long long)(1000000000.0 * aSeconds);
		std::this_thread::sleep_for(std::chrono::nanoseconds(Nanoseconds));
//		double Microseconds = 1000.0 * aSeconds;
//#if defined(_WIN32) || defined(_WIN64)
//		DWORD Duration = (DWORD)std::floor(Microseconds);
//		Sleep(Duration);
//#elif defined(__APPLE__) || defined(MACOSX)
//
//#elif defined(__linux__) && !defined(__ANDROID__)
//		int Duration = (int)std::floor(Microseconds);
//		usleep(Duration);
//#endif
	}

	void system_window::poll_input() {
		glfwPollEvents();
	}

	system_window::create_info::create_info() {
		Property = window::property();
		Swapchain = core::gcl::swapchain::property();
		FrameRate = 60.0f;
	}

	system_window::system_window(
		std::shared_ptr<gcl::context> aContext, 
		std::shared_ptr<system_display> aDisplay, 
		std::string aName, 
		const create_info& aCreateInfo, 
		math::vec<int, 2> aPosition, 
		math::vec<int, 2> aSize
	) : window(
		aContext, 
		nullptr, 
		aName, 
		aCreateInfo.Swapchain.PixelFormat,
		math::vec<uint, 3>(aSize[0], aSize[1], 1u), 
		aCreateInfo.FrameRate, 
		aCreateInfo.Swapchain.FrameCount, 
		1u
	) {
		VkResult Result = VK_SUCCESS;
		core::math::vec<uint, 3> FramebufferResolution;

		// Create GLFW System Window.
		this->WindowHandle = this->create_window_handle(aCreateInfo.Property, aSize[0], aSize[1], aName.c_str(), NULL, NULL);

		if (this->WindowHandle != NULL) {
			// Get Framebuffer Size.
			int FramebufferWidth, FramebufferHeight;
			glfwGetFramebufferSize(this->WindowHandle, &FramebufferWidth, &FramebufferHeight);
			FramebufferResolution = core::math::vec<uint, 3>(FramebufferWidth, FramebufferHeight, 1u);
		}

		// Create Vulkan Surface from GLFW window.
		if (this->WindowHandle != NULL) {
			Result = glfwCreateWindowSurface(aContext->Device->Engine->Handle, this->WindowHandle, NULL, &this->SurfaceHandle);	
		}

		// Create Swapchain from Vulkan Surface.
		if (Result == VK_SUCCESS) {
			std::shared_ptr<core::gcl::swapchain> Swapchain = std::make_shared<core::gcl::swapchain>(aContext, this->SurfaceHandle, aCreateInfo.Swapchain);
			this->Framechain = std::dynamic_pointer_cast<core::gcl::framechain>(Swapchain);
		}

		// Setup next image semaphore.
		std::vector<VkSemaphore> NextImageSemaphoreList = aContext->create_semaphore(this->Framechain->Image.size(), 0);
		for (size_t i = 0; i < NextImageSemaphoreList.size(); i++) {
			this->NextImageSemaphore.push(NextImageSemaphoreList[i]);
		}

		// Setup transition commands.
		// TODO: Free command buffers later.
		this->PredrawFrameTransition = std::vector<core::gcl::command_batch>(this->Framechain->Image.size());
		this->PostdrawFrameTransition = std::vector<core::gcl::command_batch>(this->Framechain->Image.size());
		for (size_t i = 0; i < this->Framechain->Image.size(); i++) {
			// Prepare Predraw transition commands.
			VkCommandBuffer PredrawFrameTransitionCmd = aContext->allocate_command_buffer(device::operation::GRAPHICS_AND_COMPUTE);
			aContext->begin(PredrawFrameTransitionCmd);
			// Transition image from present to shader read only optimal.
			this->Framechain->Image[i]["Color"]->transition(PredrawFrameTransitionCmd, image::layout::PRESENT_SRC_KHR, image::layout::SHADER_READ_ONLY_OPTIMAL);
			// Clear swapchain image.
			this->Framechain->Image[i]["Color"]->clear(PredrawFrameTransitionCmd, { 0.0f, 0.0f, 0.0f, 1.0f });
			aContext->end(PredrawFrameTransitionCmd);
			this->PredrawFrameTransition[i] += PredrawFrameTransitionCmd;

			// Prepare Postdraw transition commands.
			VkCommandBuffer PostdrawFrameTransitionCmd = aContext->allocate_command_buffer(device::operation::GRAPHICS_AND_COMPUTE);
			aContext->begin(PostdrawFrameTransitionCmd);
			this->Framechain->Image[i]["Color"]->transition(PostdrawFrameTransitionCmd, image::layout::SHADER_READ_ONLY_OPTIMAL, image::layout::PRESENT_SRC_KHR);
			aContext->end(PostdrawFrameTransitionCmd);
			this->PostdrawFrameTransition[i] += PostdrawFrameTransitionCmd;
		}

	}

	system_window::system_window(std::shared_ptr<core::gcl::context> aContext, std::shared_ptr<system_display> aDisplay, std::string aName, const create_info& aCreateInfo, core::math::vec<float, 3> aPosition, core::math::vec<float, 2> aSize) :
		system_window(aContext, aDisplay, aName, aCreateInfo, core::math::vec<int, 2>(aPosition[0], aPosition[1]), core::math::vec<int, 2>(aSize[0], aSize[1])) {

	}

	system_window::~system_window() {
		// Destroy Semaphores.
		while (!this->NextImageSemaphore.empty()) {
			VkSemaphore Semaphore = this->NextImageSemaphore.front();
			this->NextImageSemaphore.pop();
			this->Context->destroy_semaphore(Semaphore);
		}
		// Destroy Swapchain.
		this->Framechain = nullptr;
		// Destroy Vulkan Surface.
		vkDestroySurfaceKHR(this->Context->Device->Engine->Handle, this->SurfaceHandle, NULL);
		// Destroy GLFW Window.
		this->destroy_window_handle(this->WindowHandle);
	}

	VkResult system_window::next_frame(VkSemaphore aSemaphore, VkFence aFence) {
		VkResult ReturnValue = VK_SUCCESS;
		std::shared_ptr<core::gcl::swapchain> Swapchain = std::dynamic_pointer_cast<core::gcl::swapchain>(this->Framechain);
		this->Framechain = nullptr;
		Swapchain->ReadIndex = Swapchain->DrawIndex;
		while (true) {
			VkResult Result = VK_SUCCESS;

			// Acquire next image from swapchain.
			Result = vkAcquireNextImageKHR(this->Context->Handle, Swapchain->Handle, UINT64_MAX, aSemaphore, aFence, &Swapchain->DrawIndex);

			// If image succussfully acquired, break loop.
			if (Result == VK_SUCCESS) break;

			// Check if window has resized.
			if ((Result == VK_ERROR_OUT_OF_DATE_KHR) || (Result == VK_SUBOPTIMAL_KHR)) {
				ReturnValue = Result;
				// Create new swap chain.
				gcl::swapchain::property NewProperty = gcl::swapchain::property(Swapchain->CreateInfo, Swapchain->FrameRate);
				Swapchain = std::make_shared<core::gcl::swapchain>(this->Context, this->SurfaceHandle, NewProperty, Swapchain->Handle);
				// Also reset fence.
				if (aFence != VK_NULL_HANDLE) {
					Result = this->Context->wait_and_reset(aFence);
				}
			}
			else {
				ReturnValue = Result;
			}
		}
		this->Framechain = std::dynamic_pointer_cast<core::gcl::framechain>(Swapchain);
		return ReturnValue;
	}

	void system_window::update(double aDeltaTime, core::math::vec<float, 3> aAppliedForce, core::math::vec<float, 3> aAppliedTorque) {
		this->InputState.Mouse.update(aDeltaTime);
		this->Time += aDeltaTime;
	}

	core::gcl::command_batch system_window::next_frame(std::shared_ptr<core::gcl::semaphore_pool> aSemaphorePool) {
		VkSemaphore Semaphore = this->NextImageSemaphore.front();
		this->NextImageSemaphore.pop();
		this->NextImageSemaphore.push(Semaphore);
		// Acquire next image.
		this->next_frame(Semaphore);
		core::gcl::command_batch PredrawTransition = this->PredrawFrameTransition[this->Framechain->DrawIndex];
		PredrawTransition.WaitSemaphoreList = { Semaphore };
		PredrawTransition.WaitStageList = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		return PredrawTransition;
	}

	std::vector<core::gcl::command_batch> system_window::present_frame(std::shared_ptr<core::gcl::semaphore_pool> aSemaphorePool) {
		std::shared_ptr<core::gcl::swapchain> Swapchain = std::dynamic_pointer_cast<core::gcl::swapchain>(this->Framechain);
		core::gcl::command_batch PostdrawTransition = this->PostdrawFrameTransition[Swapchain->DrawIndex];
		core::gcl::command_batch PresentInfo;
		PresentInfo.Swapchain = { Swapchain->Handle };
		PresentInfo.ImageIndex = { Swapchain->DrawIndex };
		return { PostdrawTransition, PresentInfo };
	}

	GLFWwindow* system_window::create_window_handle(window::property aSetting, int aWidth, int aHeight, const char* aTitle, GLFWmonitor* aMonitor, GLFWwindow* aWindow) {
		glfwWindowHint(GLFW_RESIZABLE,				aSetting.Resizable);
		glfwWindowHint(GLFW_DECORATED,				aSetting.Decorated);
		glfwWindowHint(GLFW_FOCUSED,				aSetting.UserFocused);
		glfwWindowHint(GLFW_AUTO_ICONIFY,			aSetting.AutoMinimize);
		glfwWindowHint(GLFW_FLOATING,				aSetting.Floating);
		glfwWindowHint(GLFW_MAXIMIZED,				aSetting.Maximized);
		glfwWindowHint(GLFW_VISIBLE,				aSetting.Visible);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR,		aSetting.ScaleToMonitor);
		glfwWindowHint(GLFW_CENTER_CURSOR,			aSetting.CenterCursor);
		glfwWindowHint(GLFW_FOCUS_ON_SHOW,			aSetting.FocusOnShow);
		glfwWindowHint(GLFW_CLIENT_API,				GLFW_NO_API);
		glfwWindowHint(GLFW_REFRESH_RATE,			GLFW_DONT_CARE);
		GLFWwindow* ReturnHandle = glfwCreateWindow(aWidth, aHeight, aTitle, aMonitor, aWindow);
		if (ReturnHandle != NULL) {
			// User pointer to forward input stream.
			glfwSetWindowUserPointer(ReturnHandle, (void*)this);

			// system_window callbacks
			glfwSetWindowPosCallback(ReturnHandle, system_window::position_callback);
			glfwSetWindowSizeCallback(ReturnHandle, system_window::size_callback);
			glfwSetWindowCloseCallback(ReturnHandle, system_window::close_callback);
			glfwSetWindowRefreshCallback(ReturnHandle, system_window::refresh_callback);
			glfwSetWindowFocusCallback(ReturnHandle, system_window::focus_callback);
			glfwSetWindowIconifyCallback(ReturnHandle, system_window::iconify_callback);
			glfwSetWindowMaximizeCallback(ReturnHandle, system_window::maximize_callback);
			glfwSetWindowContentScaleCallback(ReturnHandle, system_window::content_scale_callback);

			// framebuffer callbacks
			glfwSetFramebufferSizeCallback(ReturnHandle, system_window::framebuffer_size_callback);

			// Mouse callbacks
			glfwSetMouseButtonCallback(ReturnHandle, system_window::mouse_button_callback);
			glfwSetCursorPosCallback(ReturnHandle, system_window::cursor_position_callback);
			glfwSetCursorEnterCallback(ReturnHandle, system_window::cursor_enter_callback);
			glfwSetScrollCallback(ReturnHandle, system_window::scroll_callback);

			// Keyboard callbacks
			glfwSetKeyCallback(ReturnHandle, system_window::key_callback);
			glfwSetCharCallback(ReturnHandle, system_window::character_callback);

			// File drop
			glfwSetDropCallback(ReturnHandle, system_window::file_drop_callback);
		}
		glfwSetInputMode(ReturnHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		return ReturnHandle;
	}

	void system_window::destroy_window_handle(GLFWwindow* aWindowHandle) {

	}

	// ------------------------------ callback methods ----------------------------- //

	void system_window::position_callback(GLFWwindow* aWindowHandle, int aPosX, int aPosY) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);
		// Window->Position		= Window->Display->convert_to_display_position(aPosX, aPosY);
		// Window->PositionVSC		= math::vec<int, 2>(aPosX, aPosY);
	}

	void system_window::size_callback(GLFWwindow* aWindowHandle, int aSizeX, int aSizeY) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);
		// Window->Size			= Window->Display->convert_to_physical_size(math::vec<int, 2>(aSizeX, aSizeY));
		// Window->SizeVSC			= math::vec<int, 2>(aSizeX, aSizeY);
	}

	void system_window::close_callback(GLFWwindow* aWindowHandle) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);
		// Window->Setting.ShouldClose = GLFW_TRUE;
	}

	// 
	void system_window::refresh_callback(GLFWwindow* aWindowHandle) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);

	}

	void system_window::focus_callback(GLFWwindow* aWindowHandle, int aFocused) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);

	}

	void system_window::iconify_callback(GLFWwindow* aWindowHandle, int aIconified) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);

	}

	void system_window::maximize_callback(GLFWwindow* aWindowHandle, int aMaximized) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);


	}

	void system_window::content_scale_callback(GLFWwindow* aWindowHandle, float aXScale, float aYScale) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);
	}

	// framebuffer callbacks

	void system_window::framebuffer_size_callback(GLFWwindow* aWindowHandle, int aFrameSizeX, int aFrameSizeY) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);
		//Window->FrameResolution = math::vec3<uint>(aFrameSizeX, aFrameSizeY, 1u);
		//Window->Engine->ThreadController.suspend(Window->Engine->RenderThreadID);
		//Window->recreate_swapchain(aFrameSizeX, aFrameSizeY);
		//Window->Engine->ThreadController.resume(Window->Engine->RenderThreadID);
	}

	// Mouse callbacks

	void system_window::mouse_button_callback(GLFWwindow* aWindowHandle, int aButton, int aAction, int aMods) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);
	}

	void system_window::cursor_position_callback(GLFWwindow* aWindowHandle, double aPosX, double aPosY) {
		static double Time = 0.0f;
		double DeltaTime = lgc::timer::get_time() - Time;
		Time = lgc::timer::get_time();
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);
		math::vec<double, 2> OldPosition = math::vec<double, 2>(Window->InputState.Mouse.Position[0], Window->InputState.Mouse.Position[1]);
		math::vec<double, 2> OldVelocity = math::vec<double, 2>(Window->InputState.Mouse.Velocity[0], Window->InputState.Mouse.Velocity[1]);
		math::vec<double, 2> NewPosition = { aPosX / ((double)Window->Framechain->Resolution[0]), aPosY / ((double)Window->Framechain->Resolution[1]) };
		math::vec<double, 2> NewVelocity = (NewPosition - OldPosition) / DeltaTime;
		math::vec<double, 2> NewAcceleration = (NewVelocity - OldVelocity) / DeltaTime;
		Window->InputState.Mouse.Position = math::vec<float, 2>(NewPosition[0], NewPosition[1]);
		Window->InputState.Mouse.Velocity = math::vec<float, 2>(NewVelocity[0], NewVelocity[1]);
		Window->InputState.Mouse.Acceleration = math::vec<float, 2>(NewAcceleration[0], NewAcceleration[1]);
		Window->InputState.Mouse.NewPosition = true;
		if (!Window->InputTarget.expired()) {
			// If object still exists, pass through input.
			Window->InputTarget.lock()->input(Window->InputState);
		}
	}

	void system_window::cursor_enter_callback(GLFWwindow* aWindowHandle, int aEntered) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);

	}

	void system_window::scroll_callback(GLFWwindow* aWindowHandle, double aOffsetX, double aOffsetY) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);

	}

	void system_window::key_callback(GLFWwindow* aWindowHandle, int aKey, int aScancode, int aAction, int aMods) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);
		Window->InputState.Keyboard(aKey) = { aAction, aMods, aScancode };
		if (!Window->InputTarget.expired()) {
			// If object still exists, pass through input.
			Window->InputTarget.lock()->input(Window->InputState);
		}
	}

	void system_window::character_callback(GLFWwindow* aWindowHandle, uint aCodepoint) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);

	}

	// File Drops

	void system_window::file_drop_callback(GLFWwindow* aWindowHandle, int aPathCount, const char** aPath) {
		system_window* Window = (system_window*)glfwGetWindowUserPointer(aWindowHandle);

	}

}
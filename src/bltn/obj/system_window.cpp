#include <geodesy/bltn/obj/system_window.h>

#include <GLFW/glfw3.h>

#include <algorithm>
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
	using namespace gpu;

	// // Signals to update thread to create window handle
	// // Needed backend for system window creation
	// std::thread::id system_window::MainThreadID								;
	// std::atomic<bool> system_window::WindowCreated							= false;
	// std::atomic<bool> system_window::WindowCreationRequest					= false;
	// system_window::window_creation_arguments system_window::WindowArg		= system_window::window_creation_arguments();
	// std::atomic<GLFWwindow*> system_window::ReturnWindow					= NULL;
	// std::atomic<GLFWwindow*> system_window::DestroyWindow					= NULL;

	system_window::swapchain::property::property() {
		this->FrameCount		= 3;
		this->FrameRate			= 60.0f;
		this->PixelFormat		= image::format::B8G8R8A8_UNORM; // HDR is image::format::R16G16B16A16_SFLOAT
		this->ColorSpace		= swapchain::colorspace::SRGB_NONLINEAR;
		this->ImageUsage		= image::usage::COLOR_ATTACHMENT;
		this->CompositeAlpha	= swapchain::composite::ALPHA_OPAQUE;
		this->PresentMode		= swapchain::present_mode::FIFO;
		this->Clipped			= false;
    }

	system_window::swapchain::property::property(VkSwapchainCreateInfoKHR aCreateInfo, float aFrameRate) {
		this->FrameCount 		= aCreateInfo.minImageCount;
		this->FrameRate 		= aFrameRate;
		this->PixelFormat 		= (image::format)aCreateInfo.imageFormat;
		this->ColorSpace 		= (swapchain::colorspace)aCreateInfo.imageColorSpace;
		this->ImageUsage 		= (image::usage)aCreateInfo.imageUsage;
		this->CompositeAlpha 	= (swapchain::composite)aCreateInfo.compositeAlpha;
		this->PresentMode 		= (swapchain::present_mode)aCreateInfo.presentMode;
		this->Clipped 			= aCreateInfo.clipped;		
	}

	system_window::swapchain::swapchain(std::shared_ptr<core::gpu::context> aContext, VkSurfaceKHR aSurface, const property& aProperty, VkSwapchainKHR aOldSwapchain) : framechain(aContext, aProperty.FrameRate, aProperty.FrameCount) {
		// TODO: Maybe change later to take in class?
		VkResult Result = this->create_swapchain(aContext, aSurface, aProperty, aOldSwapchain);

		// Setup next image semaphore.
		std::vector<VkSemaphore> NextImageSemaphoreList = aContext->create_semaphore(Image.size(), 0);
		for (size_t i = 0; i < NextImageSemaphoreList.size(); i++) {
			this->NextFrameSemaphoreList.push(NextImageSemaphoreList[i]);
		}
		this->PresentFrameSemaphoreList = aContext->create_semaphore(Image.size(), 0);
    }

    system_window::swapchain::~swapchain() {
		this->clear();
		// Destroy Semaphores.
		while (!this->NextFrameSemaphoreList.empty()) {
			VkSemaphore Semaphore = this->NextFrameSemaphoreList.front();
			this->NextFrameSemaphoreList.pop();
			this->Context->destroy_semaphore(Semaphore);
		}
		this->Context->destroy_semaphore(this->PresentFrameSemaphoreList);
    }

	VkImageCreateInfo system_window::swapchain::image_create_info() const {
		VkImageCreateInfo ImageCreateInfo{};
		ImageCreateInfo.sType					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageCreateInfo.pNext					= NULL;
		ImageCreateInfo.flags					= 0;
		ImageCreateInfo.imageType				= VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format					= CreateInfo.imageFormat;
		ImageCreateInfo.extent					= { CreateInfo.imageExtent.width, CreateInfo.imageExtent.height, 1u };
		ImageCreateInfo.mipLevels				= 1;
		ImageCreateInfo.arrayLayers				= CreateInfo.imageArrayLayers;
		ImageCreateInfo.samples					= (VkSampleCountFlagBits)image::sample::COUNT_1;
		ImageCreateInfo.tiling					= (VkImageTiling)image::tiling::OPTIMAL;
		ImageCreateInfo.usage					= CreateInfo.imageUsage;
		ImageCreateInfo.sharingMode				= CreateInfo.imageSharingMode;
		ImageCreateInfo.queueFamilyIndexCount	= 0;
		ImageCreateInfo.pQueueFamilyIndices		= NULL;
		ImageCreateInfo.initialLayout			= (VkImageLayout)image::LAYOUT_UNDEFINED;
		return ImageCreateInfo;
	}

	VkResult system_window::swapchain::next_frame(VkSemaphore aPresentFrameSemaphore, VkSemaphore aNextFrameSemaphore, VkFence aNextFrameFence) {
		VkResult ReturnValue = VK_SUCCESS;

		// Before acquiring next image, present current image.
		if (aPresentFrameSemaphore != VK_NULL_HANDLE){
			VkPresentInfoKHR PresentInfo{};
			PresentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			PresentInfo.pNext				= NULL;
			PresentInfo.waitSemaphoreCount	= 1;
			PresentInfo.pWaitSemaphores		= &aPresentFrameSemaphore;
			PresentInfo.swapchainCount		= 1;
			PresentInfo.pSwapchains			= &Handle;
			PresentInfo.pImageIndices		= &DrawIndex;
			// Present image to screen.
			ReturnValue = vkQueuePresentKHR(Context->Queue[device::operation::PRESENT], &PresentInfo);
		}

		// Set previous draw index for read operations.
		ReadIndex = DrawIndex;

		// Acquire new image.
		return vkAcquireNextImageKHR(Context->Handle, Handle, UINT64_MAX, aNextFrameSemaphore, aNextFrameFence, &DrawIndex);
	}

	VkResult system_window::swapchain::create_swapchain(std::shared_ptr<context> aContext, VkSurfaceKHR aSurface, const property& aProperty, VkSwapchainKHR aOldSwapchain) {
		VkResult Result = VK_SUCCESS;

        std::shared_ptr<gpu::device> aDevice                = aContext->Device;
		VkSurfaceCapabilitiesKHR SurfaceCapabilities		= aDevice->get_surface_capabilities(aSurface);
		std::vector<VkSurfaceFormatKHR> SurfaceFormat	    = aDevice->get_surface_format(aSurface);
		std::vector<VkPresentModeKHR> PresentMode		    = aDevice->get_surface_present_mode(aSurface);

        bool SurfaceFormatSupported = false;
		for (size_t i = 0; i < SurfaceFormat.size(); i++) {
			if ((SurfaceFormat[i].format == (VkFormat)aProperty.PixelFormat) && (SurfaceFormat[i].colorSpace == (VkColorSpaceKHR)aProperty.ColorSpace)) {
				SurfaceFormatSupported = true;
				break;
			}
		}

		bool PresentModeSupported = false;
		for (size_t i = 0; i < PresentMode.size(); i++) {
			if (PresentMode[i] == (VkPresentModeKHR)aProperty.PresentMode) {
				PresentModeSupported = true;
				break;
			}
		}

        bool TransformSupported = true;
		// TODO: Test for transform support.

        bool CompositeAlphaSupported = false;
		CompositeAlphaSupported = ((aProperty.CompositeAlpha & SurfaceCapabilities.supportedCompositeAlpha) == aProperty.CompositeAlpha);

		bool ImageUsageSupported = false;
		ImageUsageSupported = ((aProperty.ImageUsage & SurfaceCapabilities.supportedUsageFlags) == aProperty.ImageUsage);

		// Check is options are supported by device and surface.
		if ((!SurfaceFormatSupported) || (!PresentModeSupported) || (!TransformSupported) || (!CompositeAlphaSupported) || (!ImageUsageSupported))
            throw std::runtime_error("Swapchain Creation Failed: Unsupported Options.");
        
        Context                             = aContext;
		Surface 							= aSurface;
        CreateInfo                          = {};
    	CreateInfo.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreateInfo.pNext					= NULL;
		CreateInfo.flags					= 0;
		CreateInfo.surface					= aSurface;
		CreateInfo.minImageCount			= std::clamp(aProperty.FrameCount, SurfaceCapabilities.minImageCount, SurfaceCapabilities.maxImageCount == 0 ? UINT32_MAX : SurfaceCapabilities.maxImageCount);
		CreateInfo.imageFormat				= (VkFormat)aProperty.PixelFormat;
		CreateInfo.imageColorSpace			= (VkColorSpaceKHR)aProperty.ColorSpace;
		CreateInfo.imageExtent				= SurfaceCapabilities.currentExtent;
		CreateInfo.imageArrayLayers			= 1;
		CreateInfo.imageUsage				= aProperty.ImageUsage;
		CreateInfo.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.queueFamilyIndexCount	= 0;
		CreateInfo.pQueueFamilyIndices		= NULL;
		CreateInfo.preTransform				= SurfaceCapabilities.currentTransform;
		CreateInfo.compositeAlpha			= (VkCompositeAlphaFlagBitsKHR)aProperty.CompositeAlpha;
		CreateInfo.presentMode				= (VkPresentModeKHR)aProperty.PresentMode;
		CreateInfo.clipped					= aProperty.Clipped;
		CreateInfo.oldSwapchain				= aOldSwapchain;

		this->Resolution = { CreateInfo.imageExtent.width, CreateInfo.imageExtent.height, 1u };

        Result = vkCreateSwapchainKHR(aContext->Handle, &CreateInfo, NULL, &this->Handle);

        if (Result == VK_SUCCESS) {
            // Get swapchain images.
			uint32_t ImageCount = 0;
			Result = vkGetSwapchainImagesKHR(aContext->Handle, this->Handle, &ImageCount, NULL);
			std::vector<VkImage> ImageList(ImageCount);
			// Initialize with default constructed frame objects.
			Result = vkGetSwapchainImagesKHR(aContext->Handle, this->Handle, &ImageCount, ImageList.data());
			for (std::size_t i = 0; i < ImageList.size(); i++) {
				this->Image[i]["Color"] = std::make_shared<image>();
				this->Image[i]["Color"]->Context = aContext;
				this->Image[i]["Color"]->CreateInfo = this->image_create_info();
				this->Image[i]["Color"]->Handle = ImageList[i];
				this->Image[i]["Color"]->transition(image::layout::LAYOUT_UNDEFINED, image::layout::PRESENT_SRC_KHR);
				this->Image[i]["Color"]->View = this->Image[i]["Color"]->view();
			}
        }

		// Setup transition commands.
		// TODO: Free command buffers later.
		PredrawFrameOperation = std::vector<command_batch>(Image.size());
		PostdrawFrameOperation = std::vector<command_batch>(Image.size());
		for (size_t i = 0; i < Image.size(); i++) {
			// Prepare Predraw transition commands.
			VkCommandBuffer PredrawFrameTransitionCmd = aContext->allocate_command_buffer(device::operation::GRAPHICS_AND_COMPUTE);
			aContext->begin(PredrawFrameTransitionCmd);
			// Transition image from present to shader read only optimal.
			Image[i]["Color"]->transition(PredrawFrameTransitionCmd, image::layout::PRESENT_SRC_KHR, image::layout::SHADER_READ_ONLY_OPTIMAL);
			// Clear swapchain image.
			Image[i]["Color"]->clear(PredrawFrameTransitionCmd, { 0.0f, 0.0f, 0.0f, 1.0f });
			aContext->end(PredrawFrameTransitionCmd);
			PredrawFrameOperation[i] += PredrawFrameTransitionCmd;

			// Prepare Postdraw transition commands.
			VkCommandBuffer PostdrawFrameTransitionCmd = aContext->allocate_command_buffer(device::operation::GRAPHICS_AND_COMPUTE);
			aContext->begin(PostdrawFrameTransitionCmd);
			Image[i]["Color"]->transition(PostdrawFrameTransitionCmd, image::layout::SHADER_READ_ONLY_OPTIMAL, image::layout::PRESENT_SRC_KHR);
			aContext->end(PostdrawFrameTransitionCmd);
			PostdrawFrameOperation[i] += PostdrawFrameTransitionCmd;
		}

		return Result;
	}

	void system_window::swapchain::clear() {
		for (size_t i = 0; i < this->Image.size(); i++) {
			if (this->Image[i]["Color"]->View != VK_NULL_HANDLE) {
				vkDestroyImageView(this->Context->Handle, this->Image[i]["Color"]->View, NULL);
				this->Image[i]["Color"]->View = VK_NULL_HANDLE;
			}
			this->Image[i]["Color"]->Handle = VK_NULL_HANDLE;
		}
        vkDestroySwapchainKHR(this->Context->Handle, this->Handle, NULL);
	}

	bool system_window::initialize() {
		return (glfwInit() == GLFW_TRUE);
	}

	void system_window::terminate() {
		glfwTerminate();
	}

	std::set<std::string> system_window::engine_extensions() {
		std::set<std::string> EngineExtension;
		uint32_t EC = 0;
		const char** EL = glfwGetRequiredInstanceExtensions(&EC);
		for (uint32_t i = 0; i < EC; i++) {
			EngineExtension.insert(EL[i]);
		}
		return EngineExtension;
	}

	std::set<std::string> system_window::context_extensions() {
		return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	}

	void system_window::check_present_support(core::gpu::device* aDevice) {
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
				aDevice->QueueFamilyProperty[i].OperationBitfield |= core::gpu::device::operation::PRESENT;
				aDevice->QueueFamilyProperty[i].OperationList.push_back(core::gpu::device::operation::PRESENT);
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

	system_window::creator::creator() {		
		this->Display			= nullptr;
		this->ColorSpace		= swapchain::colorspace::SRGB_NONLINEAR;
		this->CompositeAlpha	= swapchain::composite::ALPHA_OPAQUE;
		this->PresentMode		= swapchain::present_mode::FIFO;
		this->Clipped			= true;
	}

	system_window::system_window(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aSystemWindowCreator) : window(aContext, aStage, aSystemWindowCreator) {
		VkResult Result = VK_SUCCESS;

		// Create GLFW System Window.
		this->WindowHandle = this->create_window_handle(aSystemWindowCreator, aSystemWindowCreator->Resolution[0], aSystemWindowCreator->Resolution[1], this->Name.c_str(), NULL, NULL);

		if (this->WindowHandle != NULL) {
			// Get Framebuffer Size.
			int FramebufferWidth, FramebufferHeight;
			glfwGetFramebufferSize(this->WindowHandle, &FramebufferWidth, &FramebufferHeight);
			// this->Resolution = core::math::vec<uint, 3>(FramebufferWidth, FramebufferHeight, 1u);
		}

		// Create Vulkan Surface from GLFW window.
		if (this->WindowHandle != NULL) {
			Result = glfwCreateWindowSurface(aContext->Device->Engine->Handle, this->WindowHandle, NULL, &this->SurfaceHandle);	
		}

		// Create Swapchain from Vulkan Surface.
		if (Result == VK_SUCCESS) {
			swapchain::property SwapchainProperty;
			SwapchainProperty.FrameCount		= aSystemWindowCreator->FrameCount;
			SwapchainProperty.FrameRate			= aSystemWindowCreator->FrameRate;
			SwapchainProperty.PixelFormat		= aSystemWindowCreator->PixelFormat;
			SwapchainProperty.ColorSpace		= aSystemWindowCreator->ColorSpace;
			SwapchainProperty.ImageUsage		= aSystemWindowCreator->ImageUsage;
			SwapchainProperty.CompositeAlpha	= aSystemWindowCreator->CompositeAlpha;
			SwapchainProperty.PresentMode		= aSystemWindowCreator->PresentMode;
			SwapchainProperty.Clipped			= aSystemWindowCreator->Clipped;
			std::shared_ptr<swapchain> Swapchain(new swapchain(aContext, this->SurfaceHandle, SwapchainProperty));
			this->Framechain = std::dynamic_pointer_cast<framechain>(Swapchain);
		}

	}

	system_window::~system_window() {
		// Destroy Swapchain.
		this->Framechain = nullptr;
		// Destroy Vulkan Surface.
		vkDestroySurfaceKHR(this->Context->Device->Engine->Handle, this->SurfaceHandle, NULL);
		// Destroy GLFW Window.
		this->destroy_window_handle(this->WindowHandle);
	}

	void system_window::update(
		double 									aDeltaTime, 
		double 									aTime, 
		const std::vector<float>& 				aAnimationWeight, 
		const std::vector<phys::animation>& 	aPlaybackAnimation,
		const std::vector<phys::force>& 		aAppliedForces
	) {
		this->InputState.Mouse.update(aDeltaTime);
	}

	core::gpu::submission_batch system_window::render(runtime::stage* aStage) {
		VkResult Result = VK_SUCCESS;
		// The next frame operation will both present previously drawn frame and acquire next
		// frame. 

		// Get temp swapchain pointer handle.
		swapchain *Swapchain = dynamic_cast<swapchain*>(this->Framechain.get());

		// Cast to rasterizer.
		std::shared_ptr<pipeline::rasterizer> Rasterizer = std::dynamic_pointer_cast<pipeline::rasterizer>(this->Pipeline[0]->CreateInfo);

		// Get next frame semaphore from queue.
		this->NextFrameSemaphore = Swapchain->NextFrameSemaphoreList.front();

		// Get next frame only if resolution is valid.
		if ((this->Framechain->Resolution[0] > 0) && (this->Framechain->Resolution[1] > 0)) {
			Result = this->Framechain->next_frame(this->PresentFrameSemaphore, this->NextFrameSemaphore);
		}

		// Rebuild pipelines and command buffers if out of date.
		if (
			// Check if swapchain is out of date.
			(Result != VK_ERROR_OUT_OF_DATE_KHR) && (Result != VK_SUBOPTIMAL_KHR) 
			&& 
			// Check if resolution is valid.
			(this->Framechain->Resolution[0] > 0) && (this->Framechain->Resolution[1] > 0) 
			&&
			// Check if pipeline resolution matches swapchain resolution.
			(this->Framechain->Resolution[0] == Rasterizer->Resolution[0]) && (this->Framechain->Resolution[1] == Rasterizer->Resolution[1]) 
		) {
			// ------------------------------ Rendering Operations ----------------------------- //

			// Keep next frame semaphore from queue and place back.
			Swapchain->NextFrameSemaphoreList.pop();
			Swapchain->NextFrameSemaphoreList.push(this->NextFrameSemaphore);
	
			// Acquire present semaphore from vector.
			this->PresentFrameSemaphore = Swapchain->PresentFrameSemaphoreList[Swapchain->DrawIndex];
	
			// Acquire predraw rendering operations.
			this->RenderingOperations += this->Framechain->predraw();
	
			// Iterate through all objects in the stage.
			gpu::command_batch StageCommandBatch;
			for (size_t i = 0; i < aStage->Object.size(); i++) {
				// Draw object.
				std::vector<std::shared_ptr<object::draw_call>> ObjectDrawCall = aStage->Object[i]->draw(this);
				std::vector<VkCommandBuffer> ObjectDrawCommand(ObjectDrawCall.size());
				for (size_t j = 0; j < ObjectDrawCall.size(); j++) {
					ObjectDrawCommand[j] = ObjectDrawCall[j]->DrawCommand;
				}
				// Group into single submission.
				StageCommandBatch += ObjectDrawCommand;
			}
	
			// Aggregate all rendering operations to subject.
			this->RenderingOperations += StageCommandBatch;
	
			// Acquire image transition and present frame if exists.
			this->RenderingOperations += this->Framechain->postdraw();
	
			// Setup safety dependencies for default rendering system.
			for (size_t i = 0; i < this->RenderingOperations.size() - 1; i++) {
				VkPipelineStageFlags Stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				this->RenderingOperations[i + 1].depends_on(this->SemaphorePool, Stage, this->RenderingOperations[i]);
			}
			
			// If system_window, make sure that next image semaphore is provided to rendering operations.
			if (this->NextFrameSemaphore != VK_NULL_HANDLE) {
				this->RenderingOperations.front().WaitStageList = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
				this->RenderingOperations.front().WaitSemaphoreList = { this->NextFrameSemaphore };
			}

			// If system_window, make sure that present semaphore is signaled after rendering operations.
			if  (this->PresentFrameSemaphore != VK_NULL_HANDLE) {
				this->RenderingOperations.back().SignalSemaphoreList = { this->PresentFrameSemaphore };
			}
		}
		else {
			// ------------------------------ Swapchain Recreate ----------------------------- //

			// Clear out all rendering commands first.
			vkDeviceWaitIdle(this->Context->Handle);
	
			// Create New swapchain based on old.
			if ((this->Framechain->Resolution[0] > 0) && (this->Framechain->Resolution[1] > 0)) {
				std::shared_ptr<swapchain> NewSwapchain = std::shared_ptr<swapchain>(new swapchain(this->Context, this->SurfaceHandle, swapchain::property(Swapchain->CreateInfo, Swapchain->FrameRate), Swapchain->Handle));
		
				// Use new swapchain to replace old.
				this->Framechain = std::dynamic_pointer_cast<framechain>(NewSwapchain);
		
				// Rebuild pipeline.
				if (this->Pipeline[0]->CreateInfo->BindPoint == pipeline::type::RASTERIZER) {
					// Resize rasterizer.
					Rasterizer->resize(this->Framechain->Resolution);
					// Rebuild pipeline.
					this->Pipeline[0] = this->Context->create_pipeline(Rasterizer);
				}
			}
	
			// Destroy all existing commandbuffers that reference this subject.
			for (auto& Object : aStage->Object) {
				// clear();
				if (Object->Renderer.count(this) > 0) {
					Object->Renderer.erase(this);
				}
			}

			// Since there has been a rebuild event, skip rendering.
			this->PresentFrameSemaphore = VK_NULL_HANDLE;
	
			// Skip render phase this frame. Try again next loop.
			this->RenderingOperations.clear();
		}
		return build(this->RenderingOperations);
	}

	GLFWwindow* system_window::create_window_handle(window::creator* aSetting, int aWidth, int aHeight, const char* aTitle, GLFWmonitor* aMonitor, GLFWwindow* aWindow) {
		glfwWindowHint(GLFW_RESIZABLE,				aSetting->Resizable);
		glfwWindowHint(GLFW_DECORATED,				aSetting->Decorated);
		glfwWindowHint(GLFW_FOCUSED,				aSetting->UserFocused);
		glfwWindowHint(GLFW_AUTO_ICONIFY,			aSetting->AutoMinimize);
		glfwWindowHint(GLFW_FLOATING,				aSetting->Floating);
		glfwWindowHint(GLFW_MAXIMIZED,				aSetting->Maximized);
		glfwWindowHint(GLFW_VISIBLE,				aSetting->Visible);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR,		aSetting->ScaleToMonitor);
		glfwWindowHint(GLFW_CENTER_CURSOR,			aSetting->CenterCursor);
		glfwWindowHint(GLFW_FOCUS_ON_SHOW,			aSetting->FocusOnShow);
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
		glfwDestroyWindow(aWindowHandle);
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
		Window->Framechain->Resolution = math::vec<uint, 3>(aFrameSizeX, aFrameSizeY, 1u);
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
		math::vec<double, 2> NewVelocity = normalize((NewPosition - OldPosition) / DeltaTime);
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
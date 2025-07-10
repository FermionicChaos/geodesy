#pragma once
#ifndef GEODESY_BLTN_OBJ_SYSTEM_WINDOW_H
#define GEODESY_BLTN_OBJ_SYSTEM_WINDOW_H

// Include engine base.
#include <geodesy/engine.h>

#include "window.h"
#include "system_display.h"

struct GLFWwindow;

namespace geodesy::bltn::obj {
	
	class system_window : public window {
	public:

		class swapchain : public framechain {
		public:

			enum colorspace {
				SRGB_NONLINEAR          	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
				DISPLAY_P3_NONLINEAR    	= VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
				EXTENDED_SRGB_LINEAR    	= VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,
				DISPLAY_P3_LINEAR       	= VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,
				DCI_P3_NONLINEAR        	= VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
				BT709_LINEAR            	= VK_COLOR_SPACE_BT709_LINEAR_EXT,
				BT709_NONLINEAR         	= VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
				BT2020_LINEAR           	= VK_COLOR_SPACE_BT2020_LINEAR_EXT,
				HDR10_ST2084            	= VK_COLOR_SPACE_HDR10_ST2084_EXT,
				DOLBYVISION             	= VK_COLOR_SPACE_DOLBYVISION_EXT,
				HDR10_HLG               	= VK_COLOR_SPACE_HDR10_HLG_EXT,
				ADOBERGB_LINEAR         	= VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,
				ADOBERGB_NONLINEAR      	= VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT,
				PASS_THROUGH            	= VK_COLOR_SPACE_PASS_THROUGH_EXT,
				EXTENDED_SRGB_NONLINEAR 	= VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT,
				DISPLAY_NATIVE_AMD      	= VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,
			};

			enum composite {
				ALPHA_OPAQUE          		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				ALPHA_PRE_MULTIPLIED  		= VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
				ALPHA_POST_MULTIPLIED 		= VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
				ALPHA_INHERIT         		= VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
			};

			enum present_mode {
				IMMEDIATE                 	= VK_PRESENT_MODE_IMMEDIATE_KHR,
				MAILBOX                   	= VK_PRESENT_MODE_MAILBOX_KHR,
				FIFO                      	= VK_PRESENT_MODE_FIFO_KHR,
				FIFO_RELAXED              	= VK_PRESENT_MODE_FIFO_RELAXED_KHR,
				SHARED_DEMAND_REFRESH     	= VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
				SHARED_CONTINUOUS_REFRESH 	= VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
			};

		    struct property {
				uint32_t					    FrameCount;
				float 							FrameRate;
				core::gpu::image::format 		PixelFormat;
				colorspace						ColorSpace;
		        int 							ImageUsage;
				composite 						CompositeAlpha;
				present_mode 					PresentMode;
				bool 							Clipped;
		        property();
				property(VkSwapchainCreateInfoKHR aCreateInfo, float aFrameRate);
			};

			std::queue<VkSemaphore> 				NextFrameSemaphoreList;
			std::vector<VkSemaphore> 				PresentFrameSemaphoreList;
			VkSurfaceKHR 							Surface;
			VkSwapchainCreateInfoKHR				CreateInfo;
			VkSwapchainKHR							Handle;

		    swapchain(std::shared_ptr<core::gpu::context> aContext, VkSurfaceKHR aSurface, const property& aProperty, VkSwapchainKHR aOldSwapchain = VK_NULL_HANDLE);
		    ~swapchain();

			VkImageCreateInfo image_create_info() const;

			VkResult next_frame(VkSemaphore aPresentFrameSemaphore = VK_NULL_HANDLE, VkSemaphore aNextFrameSemaphore = VK_NULL_HANDLE, VkFence aNextFrameFence = VK_NULL_HANDLE) override;

		private:

			VkResult create_swapchain(std::shared_ptr<core::gpu::context> aContext, VkSurfaceKHR aSurface, const property& aProperty, VkSwapchainKHR aOldSwapchain = VK_NULL_HANDLE);
			void clear();

		};

		struct creator : window::creator {
			std::shared_ptr<system_display> 		Display;
			swapchain::colorspace					ColorSpace;
			swapchain::composite 					CompositeAlpha;
			swapchain::present_mode 				PresentMode;
			bool 									Clipped;
			creator();
		};

		static bool initialize();
		static void terminate();
		static std::set<std::string> engine_extensions();
		static std::set<std::string> context_extensions();
		static void check_present_support(core::gpu::device* aDevice);

		// Timing
		static double get_time();
		static void set_time(double aTime);
		static void wait(double aSeconds);

		// Input
		static void poll_input();

		std::weak_ptr<object> 					InputTarget;
		core::hid::input 						InputState;
		GLFWwindow* 							WindowHandle;
		VkSurfaceKHR 							SurfaceHandle;

		system_window(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aSystemWindowCreator);
		~system_window();

		core::gpu::submission_batch render(runtime::stage* aStage) override;
		void virtual host_update(
			double 										aDeltaTime = 0.0f, 
			double 										aTime = 0.0f, 
			const std::vector<float>& 					aAnimationWeight = { 1.0f }, 
			const std::vector<core::phys::animation>& 	aPlaybackAnimation = {},
			const std::vector<core::phys::force>& 		aAppliedForces = {}
		) override;

	private:

		GLFWwindow* create_window_handle(window::creator* aSetting, int aWidth, int aHeight, const char* aTitle, GLFWmonitor* aMonitor, GLFWwindow* aWindow);
		void destroy_window_handle(GLFWwindow* aWindowHandle);

		// ------------------------------ Callbacks (Internal, Do Not Use) ------------------------------ //

		// struct window_creation_arguments {
		// 	window::property Setting;
		// 	int Width;
		// 	int Height;
		// 	const char* Title;
		// 	GLFWmonitor* Monitor;
		// 	GLFWwindow* Window;
		// 	window_creation_arguments();
		// };

		// // Signals to update thread to create window handle
		// // Needed backend for system window creation
		// static std::thread::id MainThreadID;
		// static std::atomic<bool> WindowCreated;
		// static std::atomic<bool> WindowCreationRequest;
		// static window_creation_arguments WindowArg;
		// static std::atomic<GLFWwindow*> ReturnWindow;
		// static std::atomic<GLFWwindow*> DestroyWindow;

		// Window Callbacks
		static void position_callback(GLFWwindow* aWindowHandle, int aPosX, int aPosY);
		static void size_callback(GLFWwindow* aWindowHandle, int aSizeX, int aSizeY);
		static void close_callback(GLFWwindow* aWindowHandle);
		static void refresh_callback(GLFWwindow* aWindowHandle);
		static void focus_callback(GLFWwindow* aWindowHandle, int aFocused);
		static void iconify_callback(GLFWwindow* aWindowHandle, int aIconified);
		static void maximize_callback(GLFWwindow* aWindowHandle, int aMaximized);
		static void content_scale_callback(GLFWwindow* aWindowHandle, float aXScale, float aYScale);

		// Framebuffer Callbacks
		static void framebuffer_size_callback(GLFWwindow* aWindowHandle, int aFrameSizeX, int aFrameSizeY);

		// Cursor Callback
		static void mouse_button_callback(GLFWwindow* aWindowHandle, int aButton, int aAction, int aMods);
		static void cursor_position_callback(GLFWwindow* aWindowHandle, double aPosX, double aPosY);
		static void cursor_enter_callback(GLFWwindow* aWindowHandle, int aEntered);
		static void scroll_callback(GLFWwindow* aWindowHandle, double aOffsetX, double aOffsetY);

		// Keyboard Callback
		static void key_callback(GLFWwindow* aWindowHandle, int aKey, int aScancode, int aAction, int aMods);
		static void character_callback(GLFWwindow* aWindowHandle, uint aCodepoint);

		// File drop
		static void file_drop_callback(GLFWwindow* aWindowHandle, int aPathCount, const char** aPath);

	};

}

#endif // !GEODESY_BLTN_OBJ_SYSTEM_WINDOW_H
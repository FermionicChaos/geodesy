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

		static bool initialize();
		static void terminate();
		static std::vector<const char*> engine_extensions();
		static std::vector<const char*> context_extensions();
		static void check_present_support(core::gcl::device* aDevice);

		// Timing
		static double get_time();
		static void set_time(double aTime);
		static void wait(double aSeconds);

		// Input
		static void poll_input();

		struct create_info {
			window::property 				Property;
			core::gcl::swapchain::property 	Swapchain;
			float 							FrameRate;
			create_info();
		};

		GLFWwindow* 								WindowHandle;
		VkSurfaceKHR 								SurfaceHandle;
		// core::gcl::swapchain* 						Swapchain;

		system_window(std::shared_ptr<core::gcl::context> aContext, std::shared_ptr<system_display> aDisplay, std::string aName, const create_info& aCreateInfo, core::math::vec<int, 2> aPosition, core::math::vec<int, 2> aSize);
		system_window(std::shared_ptr<core::gcl::context> aContext, std::shared_ptr<system_display> aDisplay, std::string aName, const create_info& aCreateInfo, core::math::vec<float, 3> aPosition, core::math::vec<float, 2> aSize);

		virtual VkResult next_frame(VkSemaphore aSemaphore = VK_NULL_HANDLE, VkFence aFence = VK_NULL_HANDLE) override;
		virtual VkPresentInfoKHR present_frame(const std::vector<VkSemaphore>& aWaitSemaphore = {}) override;

	private:

		GLFWwindow* create_window_handle(window::property aSetting, int aWidth, int aHeight, const char* aTitle, GLFWmonitor* aMonitor, GLFWwindow* aWindow);
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
		static void position_callback(GLFWwindow* ContextHandle, int PosX, int PosY);
		static void size_callback(GLFWwindow* ContextHandle, int SizeX, int SizeY);
		static void close_callback(GLFWwindow* ContextHandle);
		static void refresh_callback(GLFWwindow* ContextHandle);
		static void focus_callback(GLFWwindow* ContextHandle, int Focused);
		static void iconify_callback(GLFWwindow* ContextHandle, int Iconified);
		static void maximize_callback(GLFWwindow* ContextHandle, int Maximized);
		static void content_scale_callback(GLFWwindow* ContextHandle, float XScale, float YScale);

		// Framebuffer Callbacks
		static void framebuffer_size_callback(GLFWwindow* ContextHandle, int FrameSizeX, int FrameSizeY);

		// Cursor Callback
		static void mouse_button_callback(GLFWwindow* ContextHandle, int Button, int Action, int Mods);
		static void cursor_position_callback(GLFWwindow* ContextHandle, double PosX, double PosY);
		static void cursor_enter_callback(GLFWwindow* ContextHandle, int Entered);
		static void scroll_callback(GLFWwindow* ContextHandle, double OffsetX, double OffsetY);

		// Keyboard Callback
		static void key_callback(GLFWwindow* ContextHandle, int Key, int Scancode, int Action, int Mods);
		static void character_callback(GLFWwindow* ContextHandle, uint Codepoint);

		// File drop
		static void file_drop_callback(GLFWwindow* ContextHandle, int PathCount, const char** Path);

	};

}

#endif // !GEODESY_BLTN_OBJ_SYSTEM_WINDOW_H
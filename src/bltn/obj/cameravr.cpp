#include <geodesy/bltn/obj/cameravr.h>

#define XR_USE_GRAPHICS_API_VULKAN

#include <sstream>

// Include OpenXR headers
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vulkan/vulkan.h>

namespace geodesy::bltn::obj {

	XrInstance cameravr::Instance = XR_NULL_HANDLE; // OpenXR Instance handle
	// Vulkan Instance Extensions for OpenXR
	std::set<std::string> cameravr::EngineExtensionsModule = {
// 		// Platform-specific instance extensions
// #ifdef _WIN32
// 		VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,               // "VK_KHR_external_memory_win32"
// 		VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,            // "VK_KHR_external_semaphore_win32"
// 		VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,                // "VK_KHR_external_fence_win32"
// #elif defined(__ANDROID__)
// 		VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,                  // "VK_KHR_external_memory_fd"
// 		VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,               // "VK_KHR_external_semaphore_fd"
// 		VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,                   // "VK_KHR_external_fence_fd"
// #elif defined(__linux__)
// 		VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,                  // "VK_KHR_external_memory_fd"
// 		VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,               // "VK_KHR_external_semaphore_fd"
// 		VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,                   // "VK_KHR_external_fence_fd"
// #elif defined(__APPLE__)
// 		VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,                  // "VK_KHR_external_memory_fd"
// 		VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,               // "VK_KHR_external_semaphore_fd"
// 		VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,                   // "VK_KHR_external_fence_fd"
// #endif
// 		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 	// "VK_KHR_get_physical_device_properties2"
// 		VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, 		// "VK_KHR_external_memory_capabilities"
// 		VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME, 			// "VK_KHR_external_fence_capabilities"
// 		VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME 		// "VK_KHR_external_semaphore_capabilities"
#ifdef GEODESY_BUILD_DEBUG
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME 							// Add this line!
#endif
	};
	std::set<std::string> cameravr::EngineLayersModule = { 
		
	};
	std::set<std::string> cameravr::ContextExtensionsModule = {
// 		// Platform-specific device extensions
// #ifdef _WIN32
// 		VK_KHR_WIN32_KEYED_MUTEX_EXTENSION_NAME,                   // "VK_KHR_win32_keyed_mutex"
// #elif defined(__ANDROID__)
// 		VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME, // "VK_ANDROID_external_memory_android_hardware_buffer"
// 		VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,                // "VK_EXT_queue_family_foreign"
// #elif defined(__linux__)
// 		VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,             // "VK_EXT_external_memory_dma_buf"
// #elif defined(__APPLE__)
// 		VK_EXT_METAL_OBJECTS_EXTENSION_NAME,                       // "VK_EXT_metal_objects"
// 	#ifndef TARGET_OS_IOS
// 		VK_MVK_MOLTENVK_EXTENSION_NAME,                            // "VK_MVK_moltenvk"
// 	#endif
// #endif
// 		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, 				// "VK_KHR_dedicated_allocation"
// 		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, 			// "VK_KHR_get_memory_requirements2"
// 		VK_KHR_BIND_MEMORY_2_EXTENSION_NAME, 						// "VK_KHR_bind_memory2"
// 		VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, 						// "VK_KHR_external_memory"
// 		VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, 					// "VK_KHR_external_semaphore"
// 		VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME 						// "VK_KHR_external_fence"
	};
	std::set<std::string> cameravr::ContextLayersModule = {

	};

	cameravr::creator::creator() {
		this->RTTIID = cameravr::rttiid;
	}

	bool cameravr::initialize(std::set<std::string> aLayerList, std::set<std::string> aExtensionList) {
		// Convert sets to vectors for OpenXR API
		std::vector<const char*> LayerList = {

		};
		std::vector<const char*> ExtensionList = {			
			// ! Removed, will be a runtime option in the future
			// Platform-specific optional extensions
// #ifdef _WIN32
// 			// Windows - Desktop VR (Index, WMR, Quest via Link)
// 			"XR_EXT_eye_gaze_interaction",                          // Eye tracking (Varjo, some WMR)
// 			"XR_EXT_hp_mixed_reality_controller",                   // HP Reverb controllers
// 			"XR_MSFT_spatial_anchor",                               // Windows Mixed Reality anchors
// 			"XR_MSFT_hand_interaction",                             // WMR hand tracking
// 			"XR_VARJO_quad_views",                                  // Varjo mixed reality
// 			"XR_VARJO_foveated_rendering",                          // Varjo foveated rendering
// #elif defined(__ANDROID__)
// 			// Android - Mobile VR (Quest, Pico, ByteDance)
// 			"XR_FB_display_refresh_rate",                           // Quest/Meta refresh rate control
// 			"XR_FB_passthrough",                                    // Quest Pro/3 passthrough
// 			"XR_FB_color_space",                                    // Quest color management
// 			"XR_FB_hand_tracking_mesh",                             // Quest hand mesh
// 			"XR_FB_face_tracking",                                  // Quest Pro face tracking
// 			"XR_KHR_android_thread_settings",                       // Android performance
// 			"XR_PICO_controller_interaction",                       // Pico controllers
// 			"XR_HTC_vive_cosmos_controller_interaction",            // HTC Vive Focus
// 			"XR_BYTEDANCE_pico_controller",                         // ByteDance Pico controllers
// #elif defined(__linux__)
// 			// Linux - SteamVR ecosystem
// 			"XR_EXT_eye_gaze_interaction",                          // Eye tracking (Varjo on Linux)
// 			"XR_VALVE_analog_threshold",                            // Valve Index controllers
// 			"XR_HTC_vive_cosmos_controller_interaction",            // HTC Vive series
// 			"XR_VARJO_quad_views",                                  // Varjo mixed reality
// 			"XR_VARJO_foveated_rendering"                          // Varjo foveated rendering
// #elif defined(__APPLE__)
// 	#if TARGET_OS_IOS
// 			// iOS - Future Vision Pro support
// 			"XR_APPLE_vision_pro_hand_tracking",                   // Vision Pro hands (hypothetical)
// 			"XR_APPLE_vision_pro_eye_tracking",                    // Vision Pro eyes (hypothetical)
// 			"XR_EXT_eye_gaze_interaction",                          // Eye tracking
// 	#else
// 			// macOS - Limited VR support
// 			"XR_EXT_hand_tracking",                                 // Basic hand tracking
// 	#endif
// #endif
#ifdef GEODESY_BUILD_DEBUG
			"XR_EXT_debug_utils",
#endif 
			// Core required extensions
			XR_KHR_VULKAN_ENABLE_EXTENSION_NAME, 						// "XR_KHR_vulkan_enable"
			XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME, 				// "XR_KHR_composition_layer_depth"
			// XR_KHR_COMPOSITION_LAYER_COLOR_SCALE_BIAS_EXTENSION_NAME, 	// "XR_KHR_composition_layer_color_scale_bias"	
			// Universal optional extensions
			// "XR_KHR_composition_layer_cylinder", 						// Curved UI layers
			// "XR_KHR_composition_layer_equirect", 						// 360° content
			"XR_EXT_hand_tracking" 										// Hand tracking (most modern headsets)
		};
		// Add user-specified layers and extensions
		for (const auto& Layer : aLayerList) {
			LayerList.push_back(Layer.c_str());
		}
		for (const auto& Extension : aExtensionList) {
			ExtensionList.push_back(Extension.c_str());
		}
		XrResult Result = XR_SUCCESS;
		{
			XrInstanceCreateInfo InstanceCreateInfo{};
			InstanceCreateInfo.type                  = XR_TYPE_INSTANCE_CREATE_INFO;
			InstanceCreateInfo.next                  = nullptr;
			InstanceCreateInfo.createFlags           = 0;
			InstanceCreateInfo.applicationInfo       = { "Geodesy", 1, "Geodesy", 1, XR_API_VERSION_1_0 };
			InstanceCreateInfo.enabledApiLayerCount  = static_cast<uint32_t>(LayerList.size());
			InstanceCreateInfo.enabledApiLayerNames  = LayerList.empty() ? nullptr : LayerList.data();
			InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(ExtensionList.size());
			InstanceCreateInfo.enabledExtensionNames = ExtensionList.empty() ? nullptr : ExtensionList.data();
			// Create XR Instance
			Result = xrCreateInstance(&InstanceCreateInfo, &Instance);
		}

		// Load Vulkan Extensions
		if (Result == XR_SUCCESS) {
			// Get Vulkan Instance and Device Extensions. (Code Here)
			std::vector<XrSystemId> SystemIDList = {};
			{
				std::set<XrSystemId> SystemIDSet = {};
				std::vector<XrFormFactor> FormFactorList = {
					XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,    // VR headsets + mixed reality
					XR_FORM_FACTOR_HANDHELD_DISPLAY,        // Mobile AR
					// Future AR glasses form factors
				};
				for (auto FormFactor : FormFactorList) {
					XrSystemGetInfo SGI{};
					XrSystemId SystemID 	= XR_NULL_SYSTEM_ID;
					SGI.type 				= XR_TYPE_SYSTEM_GET_INFO;
					SGI.next 				= nullptr;
					SGI.formFactor 			= FormFactor;
					Result = xrGetSystem(Instance, &SGI, &SystemID);
					if (Result == XR_SUCCESS) {
						SystemIDSet.insert(SystemID);
					}
				}
				// Convert set to vector
				SystemIDList = std::vector<XrSystemId>(SystemIDSet.begin(), SystemIDSet.end());
			}

			// If no systems found, return failure
			if (SystemIDList.empty()) {
				cameravr::terminate();
				return false;
			}

			// Get Vulkan Instance Extensions
			PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR = nullptr;
			Result = xrGetInstanceProcAddr(Instance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanInstanceExtensionsKHR));

			// Check if function pointer is valid
			if (Result != XR_SUCCESS || xrGetVulkanInstanceExtensionsKHR == nullptr) {
				cameravr::terminate();
				return false;
			}

			// Load Vulkan Instance Extension pointer 
			PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR = nullptr;
			Result = xrGetInstanceProcAddr(Instance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanDeviceExtensionsKHR));

			// Check if function pointer is valid
			if (Result != XR_SUCCESS || xrGetVulkanDeviceExtensionsKHR == nullptr) {
				cameravr::terminate();
				return false;
			}

			// Get Instance Extensions
			for (const auto& SystemID : SystemIDList) {
				uint32_t CharacterCount = 0;
				Result = xrGetVulkanInstanceExtensionsKHR(Instance, SystemID, 0, &CharacterCount, nullptr);
				std::vector<char> VulkanInstanceExtensionList(CharacterCount, '\0');
				if (Result == XR_SUCCESS) {
					Result = xrGetVulkanInstanceExtensionsKHR(Instance, SystemID, CharacterCount, &CharacterCount, VulkanInstanceExtensionList.data());
				}
				// Parse and separate extensions
				if (Result == XR_SUCCESS) {
                    // Parse space-separated extension names and add to EngineExtensionsModule
                    std::stringstream StringStream(VulkanInstanceExtensionList.data());
                    std::string SingleExtension;
                    while (std::getline(StringStream, SingleExtension, ' ')) {
                        if (!SingleExtension.empty()) {
                            cameravr::EngineExtensionsModule.insert(SingleExtension);
                        }
                    }
				}
			}

			// Get Device Extensions
			for (const auto& SystemID : SystemIDList) {
				uint32_t CharacterCount = 0;
				Result = xrGetVulkanDeviceExtensionsKHR(Instance, SystemID, 0, &CharacterCount, nullptr);
				std::vector<char> VulkanDeviceExtensionList(CharacterCount, '\0');
				if (Result == XR_SUCCESS) {
					Result = xrGetVulkanDeviceExtensionsKHR(Instance, SystemID, CharacterCount, &CharacterCount, VulkanDeviceExtensionList.data());
				}
				// Parse and separate extensions
				if (Result == XR_SUCCESS) {
                    // Parse space-separated extension names and add to EngineExtensionsModule
                    std::stringstream StringStream(VulkanDeviceExtensionList.data());
                    std::string SingleExtension;
                    while (std::getline(StringStream, SingleExtension, ' ')) {
                        if (!SingleExtension.empty()) {
                            cameravr::ContextExtensionsModule.insert(SingleExtension);
                        }
                    }
				}
			}

		}

		// Check Result
		return (Result == XR_SUCCESS);
	}

	void cameravr::terminate() {
		xrDestroyInstance(Instance);
		Instance = XR_NULL_HANDLE;
	}

	cameravr::cameravr(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aCreator) : runtime::subject(aContext, aStage, aCreator) {
		// XrResult Result = XR_SUCCESS;
		// XrInstance Instance = XR_NULL_HANDLE;
		// XrSystemId SystemID = XR_NULL_SYSTEM_ID;
		// XrSession Session = XR_NULL_HANDLE;
		// std::vector<XrViewConfigurationView> Views;
		// XrSwapchain Swapchain = XR_NULL_HANDLE;

		// std::vector<const char*> Extension = { 
		// 	XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME
		// };

		// // Create OpenXR Instance
		// {
		// 	XrInstanceCreateInfo InstanceCreateInfo{};
		// 	InstanceCreateInfo.type 					= XR_TYPE_INSTANCE_CREATE_INFO;
		// 	InstanceCreateInfo.next 					= nullptr;
		// 	InstanceCreateInfo.createFlags 				= 0;
		// 	InstanceCreateInfo.applicationInfo 			= { "Geodesy", 1, "Geodesy", 1, XR_CURRENT_API_VERSION };
		// 	InstanceCreateInfo.enabledApiLayerCount 	= 0;
		// 	InstanceCreateInfo.enabledApiLayerNames 	= nullptr;
		// 	InstanceCreateInfo.enabledExtensionCount 	= Extension.size();
		// 	InstanceCreateInfo.enabledExtensionNames 	= Extension.data();

		// 	Result = xrCreateInstance(&InstanceCreateInfo, &Instance);
		// }
		
		// // Get OpenXR System ID.
		// {
		// 	XrSystemGetInfo SystemGetInfo{};
		// 	SystemGetInfo.type 			= XR_TYPE_SYSTEM_GET_INFO;
		// 	SystemGetInfo.next 			= nullptr;
		// 	SystemGetInfo.formFactor 	= XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
		// 	Result = xrGetSystem(Instance, &SystemGetInfo, &SystemID);
		// }

		// // Create OpenXR Session.
		// {
		// 	XrGraphicsBindingVulkan2KHR GBVK2{};
		// 	GBVK2.type 					= XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR;
		// 	GBVK2.next 					= nullptr;
		// 	GBVK2.instance 				= Engine->Handle;
		// 	GBVK2.physicalDevice 		= aContext->Device->Handle;
		// 	GBVK2.device 				= aContext->Handle;
		// 	GBVK2.queueFamilyIndex 		= aContext->Device->qfi(core::gpu::device::operation::GRAPHICS_AND_COMPUTE);
		// 	GBVK2.queueIndex 			= 0;

		// 	XrSessionCreateInfo SCI{};
		// 	SCI.type 					= XR_TYPE_SESSION_CREATE_INFO;
		// 	SCI.next 					= &GBVK2;
		// 	SCI.systemId 				= SystemID;
		// 	SCI.createFlags 			= 0;
		// 	SCI.next 					= nullptr;

		// 	Result = xrCreateSession(Instance, &SCI, &Session);
		// }

		// // Get views for each eye.
		// {
		// 	// Get views
		// 	uint32_t ViewCount = 0;
		// 	Result = xrEnumerateViewConfigurationViews(Instance, SystemID, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &ViewCount, nullptr);
		// 	Views = std::vector<XrViewConfigurationView>(ViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
		// 	Result = xrEnumerateViewConfigurationViews(Instance, SystemID, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, ViewCount, &ViewCount, Views.data());

		// }

		// // Create Swapchains for each eye.
		// {
		// 	XrSwapchainCreateInfo SCI{};
		// 	SCI.type 					= XR_TYPE_SWAPCHAIN_CREATE_INFO;
		// 	SCI.next 					= nullptr;
		// 	SCI.createFlags 			= 0;
		// 	SCI.usageFlags 				= XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		// 	SCI.format 					= VK_FORMAT_R8G8B8A8_UNORM;
		// 	SCI.sampleCount 			= 1;
		// 	SCI.width 					= 1920;
		// 	SCI.height 					= 1080;
		// 	SCI.faceCount 				= 1;
		// 	SCI.arraySize 				= 1;
		// 	SCI.mipCount 				= 1;

		// 	xrCreateSwapchain(Session, &SCI, &Swapchain);
		// }

	}

}

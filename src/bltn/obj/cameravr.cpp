#include <geodesy/bltn/obj/cameravr.h>

#define XR_USE_GRAPHICS_API_VULKAN

// Include OpenXR headers
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vulkan/vulkan.h>

namespace geodesy::bltn::obj {

	cameravr::creator::creator() {
		this->RTTIID = cameravr::rttiid;
	}

	cameravr::cameravr(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aCreator) : runtime::subject(aContext, aStage, aCreator) {
		XrResult Result = XR_SUCCESS;
		XrInstance Instance = XR_NULL_HANDLE;
		XrSystemId SystemID = XR_NULL_SYSTEM_ID;
		XrSession Session = XR_NULL_HANDLE;
		std::vector<XrViewConfigurationView> Views;
		XrSwapchain Swapchain = XR_NULL_HANDLE;

		std::vector<const char*> Extension = { 
			XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME
		};

		// Create OpenXR Instance
		{
			XrInstanceCreateInfo InstanceCreateInfo{};
			InstanceCreateInfo.type 					= XR_TYPE_INSTANCE_CREATE_INFO;
			InstanceCreateInfo.next 					= nullptr;
			InstanceCreateInfo.createFlags 				= 0;
			InstanceCreateInfo.applicationInfo 			= { "Geodesy", 1, "Geodesy", 1, XR_CURRENT_API_VERSION };
			InstanceCreateInfo.enabledApiLayerCount 	= 0;
			InstanceCreateInfo.enabledApiLayerNames 	= nullptr;
			InstanceCreateInfo.enabledExtensionCount 	= Extension.size();
			InstanceCreateInfo.enabledExtensionNames 	= Extension.data();

			Result = xrCreateInstance(&InstanceCreateInfo, &Instance);
		}
		
		// Get OpenXR System ID.
		{
			XrSystemGetInfo SystemGetInfo{};
			SystemGetInfo.type 			= XR_TYPE_SYSTEM_GET_INFO;
			SystemGetInfo.next 			= nullptr;
			SystemGetInfo.formFactor 	= XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
			Result = xrGetSystem(Instance, &SystemGetInfo, &SystemID);
		}

		// Create OpenXR Session.
		{
			XrGraphicsBindingVulkan2KHR GBVK2{};
			GBVK2.type 					= XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR;
			GBVK2.next 					= nullptr;
			GBVK2.instance 				= Engine->Handle;
			GBVK2.physicalDevice 		= aContext->Device->Handle;
			GBVK2.device 				= aContext->Handle;
			GBVK2.queueFamilyIndex 		= aContext->Device->qfi(core::gpu::device::operation::GRAPHICS_AND_COMPUTE);
			GBVK2.queueIndex 			= 0;

			XrSessionCreateInfo SCI{};
			SCI.type 					= XR_TYPE_SESSION_CREATE_INFO;
			SCI.next 					= &GBVK2;
			SCI.systemId 				= SystemID;
			SCI.createFlags 			= 0;
			SCI.next 					= nullptr;

			Result = xrCreateSession(Instance, &SCI, &Session);
		}

		// Get views for each eye.
		{
			// Get views
			uint32_t ViewCount = 0;
			Result = xrEnumerateViewConfigurationViews(Instance, SystemID, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &ViewCount, nullptr);
			Views = std::vector<XrViewConfigurationView>(ViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
			Result = xrEnumerateViewConfigurationViews(Instance, SystemID, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, ViewCount, &ViewCount, Views.data());

		}

		// Create Swapchains for each eye.
		{
			XrSwapchainCreateInfo SCI{};
			SCI.type 					= XR_TYPE_SWAPCHAIN_CREATE_INFO;
			SCI.next 					= nullptr;
			SCI.createFlags 			= 0;
			SCI.usageFlags 				= XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
			SCI.format 					= VK_FORMAT_R8G8B8A8_UNORM;
			SCI.sampleCount 			= 1;
			SCI.width 					= 1920;
			SCI.height 					= 1080;
			SCI.faceCount 				= 1;
			SCI.arraySize 				= 1;
			SCI.mipCount 				= 1;

			xrCreateSwapchain(Session, &SCI, &Swapchain);
		}

	}

}

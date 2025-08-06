#include <geodesy/engine.h>
#include <geodesy/core/gpu/device.h>

#include <geodesy/bltn/obj/system_window.h>
#include <geodesy/bltn/obj/cameravr.h>

#define XR_USE_GRAPHICS_API_VULKAN

// Include OpenXR headers
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vulkan/vulkan.h>

namespace geodesy::core::gpu {

	using namespace util;
	using namespace bltn;

	device::qfp::qfp(VkQueueFamilyProperties aQFP) {
		this->OperationBitfield     = aQFP.queueFlags;
		this->OperationList         = device::convert(this->OperationBitfield);
		this->QueueCount            = aQFP.queueCount;
	}

	void device::get_system_devices(engine* aEngine) {
		VkResult Result = VK_SUCCESS;
		VkPhysicalDevice XrPhysicalDevice = VK_NULL_HANDLE;

		// Check if OpenXR Instance is initialized, then search for preferred Vulkan graphics device
		if (bltn::obj::cameravr::Instance != XR_NULL_HANDLE) {
			XrResult Result = XR_SUCCESS;
			XrSystemId SystemID = XR_NULL_SYSTEM_ID;

			// Load Function Pointer for xrGetVulkanGraphicsDeviceKHR
			PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR = nullptr;
			Result = xrGetInstanceProcAddr(bltn::obj::cameravr::Instance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&xrGetVulkanGraphicsDeviceKHR));

			// If function pointer is available, use it to get Vulkan graphics device
			if (Result == XR_SUCCESS && xrGetVulkanGraphicsDeviceKHR != nullptr) {
				Result = xrGetVulkanGraphicsDeviceKHR(
					bltn::obj::cameravr::Instance,
					bltn::obj::cameravr::SystemID,
					aEngine->Handle,
					&XrPhysicalDevice
				);
			}
		}

		uint32_t Count = 0;
		Result = vkEnumeratePhysicalDevices(aEngine->Handle, &Count, NULL);
		if (Result != VK_SUCCESS) {
			aEngine->Logger << log::message(log::VULKAN, Result, "Error: Failed to enumerate system devices!");
			throw aEngine->Logger;
		}
		std::vector<VkPhysicalDevice> PhysicalDeviceList(Count);
		Result = vkEnumeratePhysicalDevices(aEngine->Handle, &Count, PhysicalDeviceList.data());
		if (Result != VK_SUCCESS) {
			aEngine->Logger << log::message(log::VULKAN, Result, "Error: Failed to enumerate system devices!");
			throw aEngine->Logger;
		}
		std::vector<std::shared_ptr<device>> SystemDeviceList(Count);
		for (size_t i = 0; i < PhysicalDeviceList.size(); i++) {
			SystemDeviceList[i] = std::make_shared<device>(aEngine, PhysicalDeviceList[i]);
			if (SystemDeviceList[i] == nullptr) {
				aEngine->Logger << log::message(log::GEODESY, log::ERROR, log::INSUFFICIENT_MEMORY, "Error: Failed to allocate memory for system devices!");
				throw aEngine->Logger;
			}
		}
		aEngine->Device = SystemDeviceList;
		if (XrPhysicalDevice != VK_NULL_HANDLE) {
			for (std::shared_ptr<device> Device : SystemDeviceList) {
				if (Device->Handle == XrPhysicalDevice) {
					aEngine->PrimaryDevice = Device;
					break;
				}
			}
		}
		else {
			for (std::shared_ptr<device> Device : SystemDeviceList) {
				if (Device->Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
					aEngine->PrimaryDevice = Device;
					break;
				}
			}
		}
	}

	std::vector<device::operation> device::convert(uint aOperationBitfield) {
		std::vector<operation> OperationList;
		for (int i = 0; i < 32; i++) {
			uint Value = aOperationBitfield & (1u << i);
			if (Value != 0) {
				operation Operation = (operation)Value;
				OperationList.push_back(Operation);
			}
		}
		return OperationList;
	}

	uint device::convert(std::vector<operation> aOperationList) {
		uint OperationBitfield = 0;
		for (operation Operation : aOperationList) {
			OperationBitfield |= Operation;
		}
		return OperationBitfield;
	}

	device::device(engine* aEngine, VkPhysicalDevice aPhysicalDevice) {
		this->Engine = aEngine;
		this->Handle = aPhysicalDevice;
		vkGetPhysicalDeviceFeatures(this->Handle, &this->Features);
		vkGetPhysicalDeviceProperties(this->Handle, &this->Properties);
		vkGetPhysicalDeviceMemoryProperties(this->Handle, &this->MemoryProperties);
		this->Name = this->Properties.deviceName;
		// Get Queue Family Properties
		{
			uint32_t Count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(this->Handle, &Count, NULL);
			std::vector<VkQueueFamilyProperties> QFP(Count);
			vkGetPhysicalDeviceQueueFamilyProperties(this->Handle, &Count, QFP.data());
			for (VkQueueFamilyProperties Q : QFP) {
				this->QueueFamilyProperty.push_back(Q);
			}
		}
		bltn::obj::system_window::check_present_support(this);
	}

	int device::get_memory_type_index(VkMemoryRequirements aMemoryRequirements, uint aMemoryType) const {
		int MemoryTypeIndex = -1;

		// Search for exact memory type index.
		for (uint32_t i = 0; i < MemoryProperties.memoryTypeCount; i++) {
			if (((aMemoryRequirements.memoryTypeBits & (1 << i)) >> i) && (MemoryProperties.memoryTypes[i].propertyFlags == aMemoryType)) {
				MemoryTypeIndex = i;
				break;
			}
		}

		// Search for approximate memory type index.
		if (MemoryTypeIndex == -1) {
			for (uint32_t i = 0; i < MemoryProperties.memoryTypeCount; i++) {
				if (((aMemoryRequirements.memoryTypeBits & (1 << i)) >> i) && ((MemoryProperties.memoryTypes[i].propertyFlags & aMemoryType) == aMemoryType)) {
					MemoryTypeIndex = i;
					break;
				}
			}
		}

		return MemoryTypeIndex;
	}

	int device::get_memory_type(int aMemoryTypeIndex) {
		if (aMemoryTypeIndex < 0) return 0;
		return MemoryProperties.memoryTypes[aMemoryTypeIndex].propertyFlags;
	}

	int device::qfi(uint aOperationBitfield) const {
		size_t LowestOperationCount = 32;
		int LowestOperationCountIndex = -1;
		for (size_t i = 0; i < QueueFamilyProperty.size(); i++) {
			// ((All Operations Supported) && (Lower Operation Count than currently stored))
			if (((QueueFamilyProperty[i].OperationBitfield & aOperationBitfield) == aOperationBitfield) && (LowestOperationCount > QueueFamilyProperty[i].OperationList.size())) {
				LowestOperationCount = QueueFamilyProperty[i].OperationList.size();
				LowestOperationCountIndex = i;
			}
		}
		return LowestOperationCountIndex;
	}

	int device::qfi(std::vector<operation> aOperationList) const {
		return this->qfi(device::convert(aOperationList));
	}

	VkSurfaceCapabilitiesKHR device::get_surface_capabilities(VkSurfaceKHR aSurface) {
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->Handle, aSurface, &SurfaceCapabilities);
		return SurfaceCapabilities;
	}

	std::vector<VkSurfaceFormatKHR> device::get_surface_format(VkSurfaceKHR aSurface) {
		uint32_t Count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->Handle, aSurface, &Count, NULL);
		std::vector<VkSurfaceFormatKHR> SurfaceFormatList(Count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->Handle, aSurface, &Count, SurfaceFormatList.data());
		return SurfaceFormatList;
	}

	std::vector<VkPresentModeKHR> device::get_surface_present_mode(VkSurfaceKHR aSurface) {
		uint32_t Count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->Handle, aSurface, &Count, NULL);
		std::vector<VkPresentModeKHR> PresentModeList(Count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->Handle, aSurface, &Count, PresentModeList.data());
		return PresentModeList;
	}

}

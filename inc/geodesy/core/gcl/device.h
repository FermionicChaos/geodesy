#pragma once
#ifndef GEODESY_CORE_GCL_DEVICE_H
#define GEODESY_CORE_GCL_DEVICE_H

#include "../../config.h"
#include "../math/type.h"
#include "config.h"

namespace geodesy::core::gcl {
	
	class device {
	public:

		enum operation : uint {
   			GRAPHICS       							= VK_QUEUE_GRAPHICS_BIT,
   			COMPUTE        							= VK_QUEUE_COMPUTE_BIT,
			GRAPHICS_AND_COMPUTE 					= GRAPHICS | COMPUTE,
   			TRANSFER       							= VK_QUEUE_TRANSFER_BIT,
			TRANSFER_AND_COMPUTE 					= TRANSFER | COMPUTE,
   			SPARSE_BINDING 							= VK_QUEUE_SPARSE_BINDING_BIT,
   			DRM_PROTECTED      						= VK_QUEUE_PROTECTED_BIT,
   			VIDEO_DECODE   							= VK_QUEUE_VIDEO_DECODE_BIT_KHR,
#ifdef VK_ENABLE_BETA_EXTENSIONS
			VIDEO_ENCODE 							= VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
#endif
			OPTICAL_FLOW 							= VK_QUEUE_OPTICAL_FLOW_BIT_NV,
			PRESENT 								= (1u << 31)
		};

		enum memory : uint {
      		DEVICE_LOCAL     						= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      		HOST_VISIBLE     						= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      		HOST_COHERENT    						= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      		HOST_CACHED      						= VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
      		LAZILY_ALLOCATED 						= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
      		PROTECTED        						= VK_MEMORY_PROPERTY_PROTECTED_BIT,
      		DEVICE_COHERENT  						= VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD,
      		DEVICE_UNCACHED  						= VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD,
      		RDMA_CAPABLE     						= VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV,
		};

		enum access : uint {
			INDIRECT_COMMAND_READ 					= VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
			INDEX_READ 								= VK_ACCESS_INDEX_READ_BIT,
			VERTEX_ATTRIBUTE_READ 					= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
			UNIFORM_READ 							= VK_ACCESS_UNIFORM_READ_BIT,
			INPUT_ATTACHMENT_READ 					= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
			SHADER_READ 							= VK_ACCESS_SHADER_READ_BIT,
			SHADER_WRITE 							= VK_ACCESS_SHADER_WRITE_BIT,
			COLOR_ATTACHMENT_READ 					= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			COLOR_ATTACHMENT_WRITE 					= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			DEPTH_STENCIL_ATTACHMENT_READ 			= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
			DEPTH_STENCIL_ATTACHMENT_WRITE 			= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			TRANSFER_READ 							= VK_ACCESS_TRANSFER_READ_BIT,
			TRANSFER_WRITE 							= VK_ACCESS_TRANSFER_WRITE_BIT,
			HOST_READ 								= VK_ACCESS_HOST_READ_BIT,
			HOST_WRITE 								= VK_ACCESS_HOST_WRITE_BIT,
			MEMORY_READ 							= VK_ACCESS_MEMORY_READ_BIT,
			MEMORY_WRITE 							= VK_ACCESS_MEMORY_WRITE_BIT,
			NONE 									= VK_ACCESS_NONE,
			// TRANSFORM_FEEDBACK_WRITE_EXT 										= VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
			// TRANSFORM_FEEDBACK_COUNTER_READ_EXT 								= VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
			// TRANSFORM_FEEDBACK_COUNTER_WRITE_EXT 								= VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
			// CONDITIONAL_RENDERING_READ_EXT 										= VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
			// COLOR_ATTACHMENT_READ_NONCOHERENT_EXT 								= VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
			// ACCELERATION_STRUCTURE_READ_KHR 									= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
			// ACCELERATION_STRUCTURE_WRITE_KHR 									= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
			// FRAGMENT_DENSITY_MAP_READ_EXT 										= VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
			// FRAGMENT_SHADING_RATE_ATTACHMENT_READ_KHR 							= VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR,
			// COMMAND_PREPROCESS_READ_NV 											= VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV,
			// COMMAND_PREPROCESS_WRITE_NV 										= VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV,
			// SHADING_RATE_IMAGE_READ_NV 											= VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV,
			// ACCELERATION_STRUCTURE_READ_NV 										= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV,
			// ACCELERATION_STRUCTURE_WRITE_NV 									= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV,
			// NONE_KHR 															= VK_ACCESS_NONE_KHR,
		};

		struct qfp {
			uint						OperationBitfield; 		// Contains Bitfield of Supported Operations by this Queue Family.
			std::vector<operation>		OperationList;			// Contains List of Supported Operations by this Queue Family.
			uint						QueueCount;				// Total Number of Queues in the Queue Family
			qfp(VkQueueFamilyProperties aQFP);
		};

		static void get_system_devices(engine* aEngine);
		static std::vector<operation> convert(uint aOperationBitfield);
		static uint convert(std::vector<operation> aOperationList);

		engine*									Engine;
		std::string								Name;
		VkPhysicalDevice						Handle;
		VkPhysicalDeviceFeatures 				Features;
		VkPhysicalDeviceProperties				Properties;
		VkPhysicalDeviceMemoryProperties		MemoryProperties;
		std::vector<qfp>						QueueFamilyProperty;

		device(engine* aEngine, VkPhysicalDevice aPhysicalDevice);

		int get_memory_type_index(VkMemoryRequirements aMemoryRequirements, uint aMemoryType) const;
		int get_memory_type(int aMemoryTypeIndex);

		int qfi(uint aOperationBitfield) const;
		int qfi(std::vector<operation> aOperationList) const;

		VkSurfaceCapabilitiesKHR get_surface_capabilities(VkSurfaceKHR aSurface);
		std::vector<VkSurfaceFormatKHR> get_surface_format(VkSurfaceKHR aSurface);
		std::vector<VkPresentModeKHR> get_surface_present_mode(VkSurfaceKHR aSurface);

	};

}

#endif // !GEODESY_CORE_GCL_DEVICE_H
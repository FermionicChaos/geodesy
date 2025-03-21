#pragma once
#ifndef GEODESY_CORE_GPU_BUFFER_H
#define GEODESY_CORE_GPU_BUFFER_H

/*
* Usage:
*	When using this with other functions as non pointer stack type, please pass
*	by reference/pointer. If you pass by value, the constructor/assignment methods
*	will be called and you will unintentionally create, copy and move data on the 
*	device needlessly.
* 
* TODO:
*	-Figure out how to schedule mem transfers with engine backend.
*	-Add an option to use dynamically created staging buffer.
*/

#include "../math.h"

#include "../util/variable.h"

#include "device.h"
#include "command_pool.h"

namespace geodesy::core::gpu {

	class image;

	class buffer : public std::enable_shared_from_this<buffer> {
	public:

		friend class image;

		enum usage {
			TRANSFER_SRC 												= VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			TRANSFER_DST 												= VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			UNIFORM_TEXEL 												= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
			STORAGE_TEXEL 												= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
			UNIFORM 													= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			STORAGE 													= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			INDEX 														= VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VERTEX 														= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			INDIRECT 													= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			SHADER_DEVICE_ADDRESS 										= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VIDEO_DECODE_SRC_KHR 										= VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
			VIDEO_DECODE_DST_KHR 										= VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR,
			TRANSFORM_FEEDBACK_EXT 										= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT,
			TRANSFORM_FEEDBACK_COUNTER_EXT 								= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
			CONDITIONAL_RENDERING_EXT 									= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT,
#ifdef VK_ENABLE_BETA_EXTENSIONS
			EXECUTION_GRAPH_SCRATCH_AMDX 								= VK_BUFFER_USAGE_EXECUTION_GRAPH_SCRATCH_BIT_AMDX,
#endif
			ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_KHR 			= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			ACCELERATION_STRUCTURE_STORAGE_KHR 							= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
			SHADER_BINDING_TABLE_KHR 									= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
			VIDEO_ENCODE_DST_KHR 										= VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR,
			VIDEO_ENCODE_SRC_KHR 										= VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
			SAMPLER_DESCRIPTOR_EXT 										= VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
			RESOURCE_DESCRIPTOR_EXT 									= VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
			PUSH_DESCRIPTORS_DESCRIPTOR_EXT 							= VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT,
			MICROMAP_BUILD_INPUT_READ_ONLY_EXT 							= VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT,
			MICROMAP_STORAGE_EXT 										= VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT,
			RAY_TRACING_NV 												= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
			SHADER_DEVICE_ADDRESS_EXT 									= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			SHADER_DEVICE_ADDRESS_KHR 									= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		};

		struct create_info {
			uint Memory;
			uint Usage;
			std::size_t ElementCount;
			create_info();
			create_info(uint aMemoryType, uint aBufferUsage);
		};

		std::shared_ptr<context>	Context;
		size_t 						ElementCount;

		VkBufferCreateInfo 			CreateInfo;
		VkBuffer					Handle;

		uint 						MemoryType;
		VkDeviceMemory				MemoryHandle;

		void* 						Ptr;

		buffer();
		buffer(std::shared_ptr<context> aContext, create_info aCreateInfo, int aVertexCount, util::variable aVertexLayout, void* aVertexData = NULL);
		buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, int aVertexCount, util::variable aVertexLayout, void* aVertexData = NULL);
		buffer(std::shared_ptr<context> aContext, create_info aCreateInfo, size_t aBufferSize, void* aBufferData = NULL);
		buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, size_t aBufferSize, void* aBufferData = NULL);
		buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, size_t aElementCount, size_t aBufferSize, void* aBufferData = NULL);
		~buffer();
		
		void copy(VkCommandBuffer aCommandBuffer, size_t aDestinationOffset, std::shared_ptr<buffer> aSourceData, size_t aSourceOffset, size_t aRegionSize);
		void copy(VkCommandBuffer aCommandBuffer, std::shared_ptr<buffer> aSourceData, std::vector<VkBufferCopy> aRegionList);
		void copy(VkCommandBuffer aCommandBuffer, size_t aDestinationOffset, std::shared_ptr<image> aSourceData, VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount = UINT32_MAX);
		void copy(VkCommandBuffer aCommandBuffer, std::shared_ptr<image> aSourceData, std::vector<VkBufferImageCopy> aRegionList);

		// void write(VkCommandBuffer aCommandBuffer, size_t aDestinationOffset, void* aSourceData, size_t aSourceOffset, size_t aRegionSize);
		// void write(VkCommandBuffer aCommandBuffer, void* aSourceData, std::vector<VkBufferCopy> aRegionList);
		// void read(VkCommandBuffer aCommandBuffer, size_t aSourceOffset, void* aDestinationData, size_t aDestinationOffset, size_t aRegionSize);
		// void read(VkCommandBuffer aCommandBuffer, void* aDestinationData, std::vector<VkBufferCopy> aRegionList);

		VkResult copy(size_t aDestinationOffset, std::shared_ptr<buffer> aSourceData, size_t aSourceOffset, size_t aRegionSize);
		VkResult copy(std::shared_ptr<buffer> aSourceData, std::vector<VkBufferCopy> aRegionList);
		VkResult copy(size_t aDestinationOffset, std::shared_ptr<image> aSourceData, VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount = UINT32_MAX);
		VkResult copy(std::shared_ptr<image> aSourceData, std::vector<VkBufferImageCopy> aRegionList);

		VkResult write(size_t aDestinationOffset, void* aSourceData, size_t aSourceOffset, size_t aRegionSize);
		VkResult write(void* aSourceData, std::vector<VkBufferCopy> aRegionList);
		VkResult read(size_t aSourceOffset, void* aDestinationData, size_t aDestinationOffset, size_t aRegionSize);
		VkResult read(void* aDestinationData, std::vector<VkBufferCopy> aRegionList);

		void *map_memory(size_t aOffset, size_t aSize);
		void unmap_memory();
		VkDeviceAddress device_address() const;

		VkBufferMemoryBarrier memory_barrier(
			uint aSrcAccess, uint aDstAccess,
			size_t aOffset = 0, size_t aSize = UINT32_MAX
		) const;

		VkMemoryRequirements memory_requirements() const;

	private:

		void clear();

		void zero_out();

	};

}

#endif // !GEODESY_CORE_GPU_BUFFER_H

#pragma once
#ifndef GEODESY_CORE_GCL_BUFFER_H
#define GEODESY_CORE_GCL_BUFFER_H

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
#include "context.h"
#include "command_pool.h"

namespace geodesy::core::gcl {

	class image;

	class buffer {
	public:

		friend class image;

		enum usage {
			TRANSFER_SRC			= 0x00000001,
			TRANSFER_DST			= 0x00000002,
			UNIFORM_TEXEL			= 0x00000004,
			STORAGE_TEXEL			= 0x00000008,
			UNIFORM					= 0x00000010,
			STORAGE					= 0x00000020,
			INDEX					= 0x00000040,
			VERTEX					= 0x00000080,
			INDIRECT				= 0x00000100,
			SHADER_DEVICE_ADDRESS	= 0x00020000
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

		buffer();
		buffer(std::shared_ptr<context> aContext, create_info aCreateInfo, int aVertexCount, util::variable aVertexLayout, void* aVertexData = NULL);
		buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, int aVertexCount, util::variable aVertexLayout, void* aVertexData = NULL);
		buffer(std::shared_ptr<context> aContext, create_info aCreateInfo, size_t aBufferSize, void* aBufferData = NULL);
		buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, size_t aBufferSize, void* aBufferData = NULL);
		buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, size_t aElementCount, size_t aBufferSize, void* aBufferData = NULL);
		buffer(const buffer& aInput);
		buffer(buffer&& aInput) noexcept;
		~buffer();

		buffer& operator=(const buffer& aRhs);
		buffer& operator=(buffer&& aRhs) noexcept;
		
		void copy(VkCommandBuffer aCommandBuffer, size_t aDestinationOffset, const buffer& aSourceData, size_t aSourceOffset, size_t aRegionSize);
		void copy(VkCommandBuffer aCommandBuffer, const buffer& aSourceData, std::vector<VkBufferCopy> aRegionList);
		void copy(VkCommandBuffer aCommandBuffer, size_t aDestinationOffset, const image& aSourceData, VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount = UINT32_MAX);
		void copy(VkCommandBuffer aCommandBuffer, const image& aSourceData, std::vector<VkBufferImageCopy> aRegionList);

		// void write(VkCommandBuffer aCommandBuffer, size_t aDestinationOffset, void* aSourceData, size_t aSourceOffset, size_t aRegionSize);
		// void write(VkCommandBuffer aCommandBuffer, void* aSourceData, std::vector<VkBufferCopy> aRegionList);
		// void read(VkCommandBuffer aCommandBuffer, size_t aSourceOffset, void* aDestinationData, size_t aDestinationOffset, size_t aRegionSize);
		// void read(VkCommandBuffer aCommandBuffer, void* aDestinationData, std::vector<VkBufferCopy> aRegionList);

		VkResult copy(size_t aDestinationOffset, const buffer& aSourceData, size_t aSourceOffset, size_t aRegionSize);
		VkResult copy(const buffer& aSourceData, std::vector<VkBufferCopy> aRegionList);
		VkResult copy(size_t aDestinationOffset, const image& aSourceData, VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount = UINT32_MAX);
		VkResult copy(const image& aSourceData, std::vector<VkBufferImageCopy> aRegionList);

		VkResult write(size_t aDestinationOffset, void* aSourceData, size_t aSourceOffset, size_t aRegionSize);
		VkResult write(void* aSourceData, std::vector<VkBufferCopy> aRegionList);
		VkResult read(size_t aSourceOffset, void* aDestinationData, size_t aDestinationOffset, size_t aRegionSize);
		VkResult read(void* aDestinationData, std::vector<VkBufferCopy> aRegionList);

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

#endif // !GEODESY_CORE_GCL_BUFFER_H

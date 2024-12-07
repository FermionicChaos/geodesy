#include <geodesy/core/gcl/buffer.h>

#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <vector>
#include <algorithm>

#include <geodesy/core/gcl/config.h>

// Used to interact with texture class
#include <geodesy/core/gcl/image.h>
#include <geodesy/core/gcl/context.h>

// It approximately 16 MB
#define GCL_TRANSFER_GRANULARITY_SIZE (1 << 24)

namespace geodesy::core::gcl {

	buffer::create_info::create_info() {
		this->Memory 	= (device::memory)0u;
		this->Usage 	= (buffer::usage)0u;
		this->ElementCount = 1;
	}

	buffer::create_info::create_info(uint aMemoryType, uint aBufferUsage) : create_info() {
		this->Memory = aMemoryType;
		this->Usage = aBufferUsage;
	}

	buffer::buffer() {
		this->zero_out();
	}

	buffer::buffer(std::shared_ptr<context> aContext, create_info aCreateInfo, int aVertexCount, util::variable aVertexLayout, void* aVertexData) 
	: buffer(aContext, aCreateInfo.Memory, aCreateInfo.Usage, aVertexCount * aVertexLayout.size(), aVertexData) {}

	buffer::buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, int aVertexCount, util::variable aVertexLayout, void* aVertexData)
	: buffer(aContext, aMemoryType, aBufferUsage, aVertexCount * aVertexLayout.size(), aVertexData) {}

	buffer::buffer(std::shared_ptr<context> aContext, create_info aCreateInfo, size_t aBufferSize, void* aBufferData) 
	: buffer(aContext, aCreateInfo.Memory, aCreateInfo.Usage, aCreateInfo.ElementCount, aBufferSize, aBufferData) {}

	buffer::buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, size_t aBufferSize, void* aBufferData)
	: buffer(aContext, aMemoryType, aBufferUsage, 1, aBufferSize, aBufferData) {}

	buffer::buffer(std::shared_ptr<context> aContext, uint aMemoryType, uint aBufferUsage, size_t aElementCount, size_t aBufferSize, void* aBufferData) : buffer() {
		VkResult Result = VK_SUCCESS;

		Context 								= aContext;
		ElementCount							= aElementCount;

		CreateInfo.sType						= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		CreateInfo.pNext						= NULL;
		CreateInfo.flags						= 0;
		CreateInfo.size							= aBufferSize;
		CreateInfo.usage						= (VkBufferUsageFlags)(aBufferUsage);// | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST);
		CreateInfo.sharingMode					= VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.queueFamilyIndexCount		= 0;
		CreateInfo.pQueueFamilyIndices			= NULL;

		if (Context != nullptr) {
			// Create Buffer Object
			Result = vkCreateBuffer(Context->Handle, &CreateInfo, NULL, &Handle);

			// Get Memory Requirements for Buffer.
			VkMemoryRequirements MemoryRequirements = this->memory_requirements();

			// Allocate memory for buffer.
			MemoryType = aMemoryType;
			MemoryHandle = Context->allocate_memory(MemoryRequirements, aMemoryType);

			// Bind Buffer to allocated Memory.
			Result = vkBindBufferMemory(this->Context->Handle, this->Handle, this->MemoryHandle, 0);

			// Write data to buffer object.
			if (aBufferData != NULL) {
				this->write(0, aBufferData, 0, aBufferSize);
			}
		}
	}

	buffer::buffer(const buffer& aInput) : buffer(aInput.Context, aInput.MemoryType, aInput.CreateInfo.usage, aInput.ElementCount, aInput.CreateInfo.size, NULL) {
		this->copy(0, aInput, 0, aInput.CreateInfo.size);
	}

	buffer::buffer(buffer&& aInput) noexcept {
		this->Context			= aInput.Context;
		this->CreateInfo 		= aInput.CreateInfo;
		this->Handle			= aInput.Handle;
		this->MemoryType 		= aInput.MemoryType;
		this->MemoryHandle		= aInput.MemoryHandle;
		aInput.zero_out();
	}

	buffer::~buffer() {
		this->clear();
	}

	// TODO: Optimize for memory recycling.
	buffer& buffer::operator=(const buffer& aRhs) {
		if (this == &aRhs) return *this;

		VkResult Result = VK_SUCCESS;

		this->clear();

		if (aRhs.Context != nullptr) {
			*this = buffer(aRhs.Context, aRhs.MemoryType, aRhs.CreateInfo.usage, aRhs.ElementCount, aRhs.CreateInfo.size, NULL);

			Result = this->copy(0, aRhs, 0, aRhs.CreateInfo.size);
		}

		return *this;
	}

	buffer& buffer::operator=(buffer&& aRhs) noexcept {
		this->clear();
		this->Context			= aRhs.Context;
		this->CreateInfo 		= aRhs.CreateInfo;
		this->Handle			= aRhs.Handle;
		this->MemoryType 		= aRhs.MemoryType;
		this->MemoryHandle		= aRhs.MemoryHandle;
		aRhs.zero_out();
		return *this;
	}

	void buffer::copy(VkCommandBuffer aCommandBuffer, size_t aDestinationOffset, const buffer& aSourceData, size_t aSourceOffset, size_t aRegionSize) {
		VkBufferCopy Region{};
		Region.srcOffset		= aSourceOffset;
		Region.dstOffset		= aDestinationOffset;
		Region.size				= aRegionSize;
		std::vector<VkBufferCopy> RegionList;
		RegionList.push_back(Region);
		this->copy(aCommandBuffer, aSourceData, RegionList);
	}

	void buffer::copy(VkCommandBuffer aCommandBuffer, const buffer& aSourceData, std::vector<VkBufferCopy> aRegionList) {
		vkCmdCopyBuffer(aCommandBuffer, aSourceData.Handle, this->Handle, aRegionList.size(), aRegionList.data());
	}

	void buffer::copy(VkCommandBuffer aCommandBuffer, size_t aDestinationOffset, const image& aSourceData, VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount) {
		VkBufferImageCopy Region {};
		Region.bufferOffset						= aDestinationOffset;
		Region.bufferRowLength					= 0;
		Region.bufferImageHeight				= 0;
		Region.imageSubresource.aspectMask		= image::aspect_flag(aSourceData.CreateInfo.format);
		Region.imageSubresource.mipLevel		= 0;
		Region.imageSubresource.baseArrayLayer	= aSourceArrayLayer;
		Region.imageSubresource.layerCount		= std::min(aArrayLayerCount, aSourceData.CreateInfo.arrayLayers - aSourceArrayLayer);
		std::vector<VkBufferImageCopy> RegionList = { Region };
		this->copy(aCommandBuffer, aSourceData, RegionList);
	}

	void buffer::copy(VkCommandBuffer aCommandBuffer, const image& aSourceData, std::vector<VkBufferImageCopy> aRegionList) {
		vkCmdCopyImageToBuffer(
			aCommandBuffer, 
			aSourceData.Handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			this->Handle, 
			aRegionList.size(), aRegionList.data()
		);
	}
	
	VkResult buffer::copy(size_t aDestinationOffset, const buffer& aSourceData, size_t aSourceOffset, size_t aRegionSize) {
		VkBufferCopy Region{};
		Region.srcOffset		= aSourceOffset;
		Region.dstOffset		= aDestinationOffset;
		Region.size				= aRegionSize;
		std::vector<VkBufferCopy> RegionList;
		RegionList.push_back(Region);
		return this->copy(aSourceData, RegionList);
	}

	VkResult buffer::copy(const buffer& aSourceData, std::vector<VkBufferCopy> aRegionList) {
		VkResult Result = VK_SUCCESS;
		VkCommandBuffer CommandBuffer = Context->allocate_command_buffer(device::operation::TRANSFER);
		Result = Context->begin(CommandBuffer);
		this->copy(CommandBuffer, aSourceData, aRegionList);
		Result = Context->end(CommandBuffer);
		Result = Context->execute_and_wait(device::operation::TRANSFER, CommandBuffer);
		Context->release_command_buffer(device::operation::TRANSFER, CommandBuffer);
		return Result;
	}

	VkResult buffer::copy(size_t aDestinationOffset, const image& aSourceData, VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount) {
		VkBufferImageCopy Region{};
		Region.bufferOffset						= aDestinationOffset;
		Region.bufferRowLength					= 0;
		Region.bufferImageHeight				= 0;
		Region.imageSubresource.aspectMask		= image::aspect_flag(aSourceData.CreateInfo.format);
		Region.imageSubresource.mipLevel		= 0;
		Region.imageSubresource.baseArrayLayer	= aSourceArrayLayer;
		Region.imageSubresource.layerCount		= std::min(aArrayLayerCount, aSourceData.CreateInfo.arrayLayers - aSourceArrayLayer);
		Region.imageOffset 						= aSourceOffset;
		Region.imageExtent 						= aRegionExtent;
		std::vector<VkBufferImageCopy> RegionList = { Region };
		return this->copy(aSourceData, RegionList);
	}

	VkResult buffer::copy(const image& aSourceData, std::vector<VkBufferImageCopy> aRegionList) {
		VkResult Result = VK_SUCCESS;
		VkCommandBuffer CommandBuffer = Context->allocate_command_buffer(device::operation::TRANSFER);
		Result = Context->begin(CommandBuffer);
		this->copy(CommandBuffer, aSourceData, aRegionList);
		Result = Context->end(CommandBuffer);
		Result = Context->execute_and_wait(device::operation::TRANSFER, CommandBuffer);
		Context->release_command_buffer(device::operation::TRANSFER, CommandBuffer);
		return Result;
	}

	VkResult buffer::write(size_t aDestinationOffset, void* aSourceData, size_t aSourceOffset, size_t aRegionSize) {
		std::vector<VkBufferCopy> RegionList;
		VkBufferCopy Region{ aSourceOffset, aDestinationOffset, aRegionSize };
		RegionList.push_back(Region);
		return this->write(aSourceData, RegionList);
	}

	VkResult buffer::write(void* aSourceData, std::vector<VkBufferCopy> aRegionList) {
		VkResult Result = VK_SUCCESS;
		if ((this->MemoryType & device::memory::HOST_VISIBLE) == device::memory::HOST_VISIBLE) {
			// Host Visible, can be written to directly.
			for (size_t i = 0; i < aRegionList.size(); i++) {
				// Calculate offset addresses
				void* ptr = this->map_memory(aRegionList[i].dstOffset, aRegionList[i].size);
				//uintptr_t TargetAddress = (uintptr_t)ptr + aRegionList[i].dstOffset;
				uintptr_t SourceAddress = (uintptr_t)aSourceData + aRegionList[i].srcOffset;
				// Copy specified targets.
				memcpy((void*)ptr, (void*)SourceAddress, aRegionList[i].size);
				this->unmap_memory(&ptr);
			}
		} else {
			// Not Host Visible, use staging buffer. For large uploads, we will try something different.
			// Say we have a large model, we want to use a staging buffer that is small to upload chunks
			// to device memory.
			size_t StagingBufferSize = GCL_TRANSFER_GRANULARITY_SIZE;

			// Not Host Visible, use staging buffer.
			buffer StagingBuffer(
				Context,
				device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT,
				buffer::TRANSFER_SRC,
				StagingBufferSize
			);

			// Copy each memory region to target buffer.
			for (size_t i = 0; i < aRegionList.size(); i++) {
				
				size_t Remainder = aRegionList[i].size;
				do {

					// Calculate chunk offsets for region for transfer.
					size_t ChunkOffset = aRegionList[i].size - Remainder;

					// Insure that we do not exceed the staging buffer size.
					size_t ChunkSize = std::clamp(Remainder, (size_t)0, StagingBufferSize);

					// Write data to staging buffer, if less than ChunkSize, then we are done.
					Result = StagingBuffer.write(0, aSourceData, aRegionList[i].srcOffset + ChunkOffset, ChunkSize);

					// Execute transfer operation.
					Result = this->copy(aRegionList[i].dstOffset + ChunkOffset, StagingBuffer, 0, ChunkSize);

					// Recalculate remaining data to send.
					Remainder -= ChunkSize;

				} while (Remainder > 0);

			}

		}
		return Result;
	}

	VkResult buffer::read(size_t aSourceOffset, void* aDestinationData, size_t aDestinationOffset, size_t aRegionSize) {
		std::vector<VkBufferCopy> RegionList;
		VkBufferCopy Region{ aSourceOffset, aDestinationOffset, aRegionSize };
		RegionList.push_back(Region);
		return this->read(aDestinationData, RegionList);
	}

	VkResult buffer::read(void* aDestinationData, std::vector<VkBufferCopy> aRegionList) {
		VkResult Result = VK_SUCCESS;
		if ((this->MemoryType & device::memory::HOST_VISIBLE) == device::memory::HOST_VISIBLE) {
			// Host Visible, can be written to directly.
			for (size_t i = 0; i < aRegionList.size(); i++) {
				// Calculate offset addresses
				void* ptr = this->map_memory(aRegionList[i].srcOffset, aRegionList[i].size);
				uintptr_t TargetAddress = (uintptr_t)aDestinationData + aRegionList[i].dstOffset;
				// Copy specified targets.
				memcpy((void*)TargetAddress, ptr, aRegionList[i].size);
				this->unmap_memory(&ptr);
			}
		} else {
			// Not Host Visible, use staging buffer. For large uploads, we will try something different.
			// Say we have a large model, we want to use a staging buffer that is small to upload chunks
			// to device memory.
			size_t StagingBufferSize = GCL_TRANSFER_GRANULARITY_SIZE;

			// Not Host Visible, use staging buffer.
			buffer StagingBuffer(
				Context,
				device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT,
				buffer::TRANSFER_DST,
				StagingBufferSize
			);

			// Copy each memory region to aDestinationData.
			for (size_t i = 0; i < aRegionList.size(); i++) {
				
				size_t Remainder = aRegionList[i].size;
				do {

					// Calculate chunk offsets for region for transfer.
					size_t ChunkOffset = aRegionList[i].size - Remainder;

					// Insure that we do not exceed the staging buffer size.
					size_t ChunkSize = std::clamp(Remainder, (size_t)0, StagingBufferSize);

					// Copy *this into staging buffer.
					StagingBuffer.copy(0, *this, aRegionList[i].srcOffset + ChunkOffset, ChunkSize);

					// Read staging buffer and copy into host memory aDestination Data.
					StagingBuffer.read(0, aDestinationData, aRegionList[i].dstOffset + ChunkOffset, ChunkSize);

					// Recalculate remaining data to send.
					Remainder -= ChunkSize;

				} while (Remainder > 0);

			}

		}

		return Result;
	}

	void *buffer::map_memory(size_t aOffset, size_t aSize) {
		VkResult Result = VK_SUCCESS;
		void *ptr = NULL;
		Result = vkMapMemory(this->Context->Handle, this->MemoryHandle, aOffset, aSize, 0, &ptr);
		return ptr;
	}

	void buffer::unmap_memory(void **aPtr) {
		vkUnmapMemory(this->Context->Handle, this->MemoryHandle);
		*aPtr = NULL;
	}

	VkBufferMemoryBarrier buffer::memory_barrier(
			uint aSrcAccess, uint aDstAccess,
			size_t aOffset, size_t aSize
	) const {
		VkBufferMemoryBarrier MemoryBarrier{};
		MemoryBarrier.sType						= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		MemoryBarrier.pNext						= NULL;
		MemoryBarrier.srcAccessMask				= aSrcAccess;
		MemoryBarrier.dstAccessMask				= aDstAccess;
		MemoryBarrier.srcQueueFamilyIndex		= VK_QUEUE_FAMILY_IGNORED;
		MemoryBarrier.dstQueueFamilyIndex		= VK_QUEUE_FAMILY_IGNORED;
		MemoryBarrier.buffer					= this->Handle;
		MemoryBarrier.offset					= aOffset;
		MemoryBarrier.size						= std::min(aSize, this->CreateInfo.size - aOffset);
		return MemoryBarrier;
	}

	VkMemoryRequirements buffer::memory_requirements() const {
		return this->Context->get_buffer_memory_requirements(this->Handle);
	}

	void buffer::clear() {
		if (Context != nullptr) {
			if (Handle != VK_NULL_HANDLE) {
				vkDestroyBuffer(Context->Handle, Handle, NULL);
			}
			Context->free_memory(MemoryHandle);
		}
		this->zero_out();
	}

	void buffer::zero_out() {
		this->Context		= nullptr;
		this->CreateInfo 	= {};
		this->Handle 		= VK_NULL_HANDLE;
		this->ElementCount 	= 0;
		this->MemoryType 	= 0;
		this->MemoryHandle 	= VK_NULL_HANDLE;
	}

}

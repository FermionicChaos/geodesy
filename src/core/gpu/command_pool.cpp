#include <geodesy/core/gpu/command_pool.h>
#include <geodesy/core/gpu/context.h>

#include <geodesy/core/util/log.h>

namespace geodesy::core::gpu {

	command_pool::command_pool() {
		this->Context 	= nullptr;
		this->Handle 	= VK_NULL_HANDLE;
	}

	command_pool::command_pool(std::shared_ptr<context> aContext, uint32_t aQueueFamilyIndex, VkCommandPoolCreateFlags aFlags) : command_pool() {
		VkResult Result = VK_SUCCESS;
		VkCommandPoolCreateInfo CreateInfo {};
		this->Context 					= aContext;
		CreateInfo.sType 				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CreateInfo.queueFamilyIndex 	= aQueueFamilyIndex;
		CreateInfo.flags 				= aFlags;
		Result = vkCreateCommandPool(aContext->Handle, &CreateInfo, nullptr, &Handle);
	}

	command_pool::command_pool(std::shared_ptr<context> aContext, device::operation aDeviceOperation, VkCommandPoolCreateFlags aFlags) 
		: command_pool(aContext, aContext->Device->qfi(aDeviceOperation), aFlags) 
	{}

	command_pool::~command_pool() {
		for (VkCommandBuffer Cmd : CommandBufferSet) {
			vkFreeCommandBuffers(this->Context->Handle, this->Handle, 1, &Cmd);
		}
		vkDestroyCommandPool(this->Context->Handle, this->Handle, nullptr);
	}

	VkCommandBuffer command_pool::allocate(VkCommandBufferLevel aLevel) {
		std::vector<VkCommandBuffer> CommandList = this->allocate(1, aLevel);
		return CommandList[0];
	}

	std::vector<VkCommandBuffer> command_pool::allocate(uint32_t aCount, VkCommandBufferLevel aLevel) {
		VkResult Result = VK_SUCCESS;
		VkCommandBufferAllocateInfo AllocateInfo {};
		std::vector<VkCommandBuffer> CommandList(aCount);
		AllocateInfo.sType 					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocateInfo.commandPool 			= this->Handle;
		AllocateInfo.level 					= aLevel;
		AllocateInfo.commandBufferCount 	= aCount;
		Result = vkAllocateCommandBuffers(this->Context->Handle, &AllocateInfo, CommandList.data());
		this->CommandBufferSet.insert(CommandList.begin(), CommandList.end());
		return CommandList;
	}

	void command_pool::release(VkCommandBuffer& aCommandBuffer) {
		std::vector<VkCommandBuffer> CommandList = {aCommandBuffer};
		this->release(CommandList);
		aCommandBuffer = VK_NULL_HANDLE;
	}

	void command_pool::release(std::vector<VkCommandBuffer>& aCommandBuffer) {
		for (VkCommandBuffer CommandBuffer : aCommandBuffer) {
			this->CommandBufferSet.erase(CommandBuffer);
		}
		vkFreeCommandBuffers(this->Context->Handle, this->Handle, aCommandBuffer.size(), aCommandBuffer.data());
		aCommandBuffer.clear();
	}

}
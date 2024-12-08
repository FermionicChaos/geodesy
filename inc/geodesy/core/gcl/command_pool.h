#pragma once
#ifndef GEODESY_CORE_GCL_COMMAND_POOL_H
#define GEODESY_CORE_GCL_COMMAND_POOL_H

#include "../../config.h"
#include "config.h"
#include "device.h"

namespace geodesy::core::gcl {

	class command_pool {
	public:


		std::shared_ptr<context>		Context;
		VkCommandPool 					Handle;
		std::set<VkCommandBuffer> 		CommandBufferSet;	
		
		command_pool();
		command_pool(std::shared_ptr<context> aContext, uint32_t aQueueFamilyIndex, VkCommandPoolCreateFlags aFlags = 0);
		command_pool(std::shared_ptr<context> aContext, device::operation aDeviceOperation, VkCommandPoolCreateFlags aFlags = 0);
		~command_pool();

		VkCommandBuffer allocate(VkCommandBufferLevel aLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		std::vector<VkCommandBuffer> allocate(uint32_t aCount, VkCommandBufferLevel aLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		void release(VkCommandBuffer& aCommandBuffer);
		void release(std::vector<VkCommandBuffer>& aCommandBuffer);

	};

}

#endif // !GEODESY_CORE_GCL_COMMAND_POOL_H
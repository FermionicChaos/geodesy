#pragma once
#ifndef GEODESY_CORE_GCL_COMMAND_BATCH_H
#define GEODESY_CORE_GCL_COMMAND_BATCH_H

#include "../../config.h"
#include "config.h"

namespace geodesy::core::gcl {

	class command_batch {
	public:

		std::shared_ptr<context>				Context;
		std::vector<VkSemaphore> 				WaitSemaphoreSemaphoreList;
		std::vector<VkPipelineStageFlags> 		WaitSemaphoreStageList;
		std::vector<VkCommandBuffer> 			CommandBufferList;
		std::vector<VkSemaphore> 				SignalSemaphoreList;

	};

}

#endif // !GEODESY_CORE_GCL_COMMAND_BATCH_H
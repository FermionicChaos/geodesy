#pragma once
#ifndef GEODESY_CORE_GCL_COMMAND_BATCH_H
#define GEODESY_CORE_GCL_COMMAND_BATCH_H

#include "../../config.h"
#include "config.h"
#include "semaphore_pool.h"

namespace geodesy::core::gcl {

	class command_batch {
	public:

		std::vector<VkSemaphore> 					WaitSemaphoreSemaphoreList;
		std::vector<VkPipelineStageFlags> 			WaitSemaphoreStageList;
		std::vector<VkCommandBuffer> 				CommandBufferList;
		std::vector<VkSemaphore> 					SignalSemaphoreList;

		void depends_on(std::shared_ptr<semaphore_pool> aSemaphorePool, VkPipelineStageFlags aWaitStage, command_batch& aWaitBatch);
		VkSubmitInfo build() const;

	};

}

#endif // !GEODESY_CORE_GCL_COMMAND_BATCH_H
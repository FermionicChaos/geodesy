#pragma once
#ifndef GEODESY_CORE_GPU_COMMAND_BATCH_H
#define GEODESY_CORE_GPU_COMMAND_BATCH_H

#include "../../config.h"
#include "config.h"
#include "semaphore_pool.h"

namespace geodesy::core::gpu {

	class command_batch {
	public:

		// Can either be a VkSubmitInfo, or VkPresentInfoKHR.
		std::vector<VkCommandBuffer> 				CommandBufferList;
		std::vector<VkSemaphore> 					WaitSemaphoreList;
		std::vector<VkPipelineStageFlags> 			WaitStageList;
		std::vector<VkSemaphore> 					SignalSemaphoreList;

		command_batch& operator+=(VkCommandBuffer aCommandBuffer);
		command_batch& operator+=(const std::vector<VkCommandBuffer>& aCommandBufferList);

		void depends_on(std::shared_ptr<semaphore_pool> aSemaphorePool, VkPipelineStageFlags aWaitStage, command_batch& aWaitBatch);
		VkSubmitInfo build_submit_info() const;

	};

	std::vector<command_batch> operator+(const std::vector<command_batch>& aLeftBatch, const command_batch& aRightBatch);
	std::vector<command_batch> operator+(const std::vector<command_batch>& aLeftBatch, const std::vector<command_batch>& aRightBatch);
	
	std::vector<command_batch>& operator+=(std::vector<command_batch>& aLeftBatch, const command_batch& aRightBatch);
	std::vector<command_batch>& operator+=(std::vector<command_batch>& aLeftBatch, const std::vector<command_batch>& aRightBatch);

}

#endif // !GEODESY_CORE_GPU_COMMAND_BATCH_H
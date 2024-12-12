#include <geodesy/core/gcl/command_batch.h>

namespace geodesy::core::gcl {

	void command_batch::depends_on(std::shared_ptr<semaphore_pool> aSemaphorePool, VkPipelineStageFlags aWaitStage, command_batch& aWaitBatch) {
		VkSemaphore Semaphore = aSemaphorePool->aquire();
		this->WaitSemaphoreSemaphoreList.push_back(Semaphore);
		this->WaitSemaphoreStageList.push_back(aWaitStage);
		aWaitBatch.SignalSemaphoreList.push_back(Semaphore);
	}

	VkSubmitInfo command_batch::build() const {
		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType 					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.pNext 					= NULL;
		SubmitInfo.waitSemaphoreCount 		= this->WaitSemaphoreSemaphoreList.size();
		SubmitInfo.pWaitSemaphores 			= this->WaitSemaphoreSemaphoreList.data();
		SubmitInfo.pWaitDstStageMask 		= this->WaitSemaphoreStageList.data();
		SubmitInfo.commandBufferCount 		= this->CommandBufferList.size();
		SubmitInfo.pCommandBuffers 			= this->CommandBufferList.data();
		SubmitInfo.signalSemaphoreCount 	= this->SignalSemaphoreList.size();
		SubmitInfo.pSignalSemaphores 		= this->SignalSemaphoreList.data();
		return SubmitInfo;
	}

}
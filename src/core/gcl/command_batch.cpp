#include <geodesy/core/gcl/command_batch.h>

namespace geodesy::core::gcl {

	command_batch& command_batch::operator+=(VkCommandBuffer aCommandBuffer) {
		return (*this += std::vector<VkCommandBuffer>{aCommandBuffer} );
	}

	command_batch& command_batch::operator+=(const std::vector<VkCommandBuffer>& aCommandBufferList) {
		this->CommandBufferList.insert(this->CommandBufferList.end(), aCommandBufferList.begin(), aCommandBufferList.end());
		return *this;
	}


	void command_batch::depends_on(std::shared_ptr<semaphore_pool> aSemaphorePool, VkPipelineStageFlags aWaitStage, command_batch& aWaitBatch) {
		VkSemaphore Semaphore = aSemaphorePool->acquire();
		this->WaitSemaphoreList.push_back(Semaphore);
		this->WaitStageList.push_back(aWaitStage);
		aWaitBatch.SignalSemaphoreList.push_back(Semaphore);
	}

	VkSubmitInfo command_batch::build_submit_info() const {
		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType 					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.pNext 					= NULL;
		SubmitInfo.waitSemaphoreCount 		= this->WaitSemaphoreList.size();
		SubmitInfo.pWaitSemaphores 			= this->WaitSemaphoreList.data();
		SubmitInfo.pWaitDstStageMask 		= this->WaitStageList.data();
		SubmitInfo.commandBufferCount 		= this->CommandBufferList.size();
		SubmitInfo.pCommandBuffers 			= this->CommandBufferList.data();
		SubmitInfo.signalSemaphoreCount 	= this->SignalSemaphoreList.size();
		SubmitInfo.pSignalSemaphores 		= this->SignalSemaphoreList.data();
		return SubmitInfo;
	}

	VkPresentInfoKHR command_batch::build_present_info() const {
		VkPresentInfoKHR PresentInfo{};
		PresentInfo.sType 					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		PresentInfo.pNext 					= NULL;
		PresentInfo.waitSemaphoreCount 		= this->WaitSemaphoreList.size();
		PresentInfo.pWaitSemaphores 		= this->WaitSemaphoreList.data();
		PresentInfo.swapchainCount 			= this->Swapchain.size();
		PresentInfo.pSwapchains 			= this->Swapchain.data();
		PresentInfo.pImageIndices 			= this->ImageIndex.data();
		PresentInfo.pResults 				= NULL;
		return PresentInfo;
	}

	std::vector<command_batch> operator+(const std::vector<command_batch>& aLeftBatch, const command_batch& aRightBatch) {
		return (aLeftBatch + std::vector<command_batch>{aRightBatch});
	}

	std::vector<command_batch> operator+(const std::vector<command_batch>& aLeftBatch, const std::vector<command_batch>& aRightBatch) {
		size_t LeftNonZeroCount = 0;
		size_t RightNonZeroCount = 0;
		// Determine non zero elements in both sides.
		for (size_t i = 0; i < aLeftBatch.size(); i++) {
			if ((aLeftBatch[i].CommandBufferList.size() > 0) || (aLeftBatch[i].Swapchain.size() > 0)) {
				LeftNonZeroCount++;
			}
		}
		for (size_t i = 0; i < aRightBatch.size(); i++) {
			if ((aRightBatch[i].CommandBufferList.size() > 0) || (aRightBatch[i].Swapchain.size() > 0)) {
				RightNonZeroCount++;
			}
		}
		std::vector<command_batch> Result(LeftNonZeroCount + RightNonZeroCount);
		size_t ResultIndex = 0;
		// Copy over left batch.
		for (size_t i = 0; i < aLeftBatch.size(); i++) {
			if ((aLeftBatch[i].CommandBufferList.size() > 0) || (aLeftBatch[i].Swapchain.size() > 0)) {
				Result[ResultIndex] = aLeftBatch[i];
				ResultIndex++;
			}
		}
		// Copy over right batch.
		for (size_t i = 0; i < aRightBatch.size(); i++) {
			if ((aRightBatch[i].CommandBufferList.size() > 0) || (aRightBatch[i].Swapchain.size() > 0)) {
				Result[ResultIndex] = aRightBatch[i];
				ResultIndex++;
			}
		}
    	return Result;
	}

	std::vector<command_batch>& operator+=(std::vector<command_batch>& aLeftBatch, const command_batch& aRightBatch) {
		return (aLeftBatch += std::vector<command_batch>{aRightBatch});
	}

	std::vector<command_batch>& operator+=(std::vector<command_batch>& aLeftBatch, const std::vector<command_batch>& aRightBatch) {
		aLeftBatch = (aLeftBatch + aRightBatch);
		return aLeftBatch;
	}

}
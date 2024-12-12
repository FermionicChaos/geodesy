#include <geodesy/core/gcl/submission_batch.h>

namespace geodesy::core::gcl {

	
	submission_batch& submission_batch::operator+=(const VkSubmitInfo& aSubmitInfo) {
		return (*this += std::vector<VkSubmitInfo>{aSubmitInfo});
	}

	submission_batch& submission_batch::operator+=(const std::vector<VkSubmitInfo>& aSubmitInfo) {
		// Do the same thing with submission infos, but check for non zero command buffer count.
		size_t NonZeroCount = 0;
		for (auto& SubmitInfo : aSubmitInfo) {
			if (SubmitInfo.commandBufferCount != 0) {
				NonZeroCount++;
			}
		}

		// Allocate a list of submit infos that have valid command buffers.
		std::vector<VkSubmitInfo> NewSubmitInfo(NonZeroCount);
		for (size_t i = 0, j = 0; i < aSubmitInfo.size(); i++) {
			if (aSubmitInfo[i].commandBufferCount != 0) {
				NewSubmitInfo[j] = aSubmitInfo[i];
				j++;
			}
		}

		// Add the submit info to the list.
		this->SubmitInfo.insert(this->SubmitInfo.end(), NewSubmitInfo.begin(), NewSubmitInfo.end());

		return *this;
	}

	submission_batch& submission_batch::operator+=(const VkPresentInfoKHR& aPresentInfo) {
		return (*this += std::vector<VkPresentInfoKHR>{aPresentInfo});
	}

	submission_batch& submission_batch::operator+=(const std::vector<VkPresentInfoKHR>& aPresentInfoKHR) {
		// Find empty elements and remove them.
		size_t NonZeroCount = 0;
		for (auto& PresentInfoKHR : aPresentInfoKHR) {
			if (PresentInfoKHR.swapchainCount != 0) {
				NonZeroCount++;
			}
		}

		// Allocate a list of present infos that have valid swapchains.
		std::vector<VkPresentInfoKHR> NewPresentInfoKHR(NonZeroCount);
		for (size_t i = 0, j = 0; i < aPresentInfoKHR.size(); i++) {
			if (aPresentInfoKHR[i].swapchainCount != 0) {
				NewPresentInfoKHR[j] = aPresentInfoKHR[i];
				j++;
			}
		}

		// Add the present info to the list.
		this->PresentInfo.insert(this->PresentInfo.end(), NewPresentInfoKHR.begin(), NewPresentInfoKHR.end());

		return *this;
	}

	submission_batch& submission_batch::operator+=(const submission_batch& aSubmissionBatch) {
		return (*this += std::vector<submission_batch>{aSubmissionBatch});
	}

	submission_batch& submission_batch::operator+=(const std::vector<submission_batch>& aSubmissionBatch) {
		// Condense all submission info and present info to the left.
		for (auto& SubmissionBatch : aSubmissionBatch) {
			*this += SubmissionBatch.SubmitInfo;
			*this += SubmissionBatch.PresentInfo;
		}
		return *this;
	}

	submission_batch build(const std::vector<command_batch>& aCommandBatch) {
		// TODO: figure out command_batch internals first.
		submission_batch SubmissionBatch;
		std::vector<VkSubmitInfo> SubmitInfo(aCommandBatch.size());
		for (size_t i = 0; i < aCommandBatch.size(); i++) {
			SubmitInfo[i] = aCommandBatch[i].build();
		}

		return SubmissionBatch;
	}

}

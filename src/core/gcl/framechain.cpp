#include <geodesy/core/gcl/framechain.h>

#include <geodesy/core/gcl/context.h>

namespace geodesy::core::gcl {

	framechain::framechain(std::shared_ptr<context> aContext, double aFrameRate, uint32_t aFrameCount) {
		this->DrawIndex = 0;
		this->ReadIndex = 0;
		this->FrameRate = aFrameRate;
		this->Timer = 1.0 / aFrameRate;
		this->Context = aContext;
		this->Image = std::vector<std::map<std::string, std::shared_ptr<image>>>(aFrameCount);
	}

	framechain::~framechain() {
		for (size_t i = 0; i < this->PredrawFrameOperation.size(); i++) {
			this->Context->release_command_buffer(device::operation::GRAPHICS_AND_COMPUTE, this->PredrawFrameOperation[i].CommandBufferList);
		}
		for (size_t i = 0; i < this->PostdrawFrameOperation.size(); i++) {
			this->Context->release_command_buffer(device::operation::GRAPHICS_AND_COMPUTE, this->PostdrawFrameOperation[i].CommandBufferList);
		}
	}

	std::map<std::string, std::shared_ptr<image>> framechain::read_frame() {
		return this->Image[this->ReadIndex];
	}

	std::map<std::string, std::shared_ptr<image>> framechain::draw_frame() {
		return this->Image[this->DrawIndex];
	}

	bool framechain::ready_to_render() {
		return this->Timer.check();
	}

	VkResult framechain::next_frame_now() {
		return VK_SUCCESS;
	}

	VkResult framechain::present_frame_now() {
		return VK_SUCCESS;
	}

	VkResult framechain::next_frame(VkSemaphore aPresentFrameSemaphore, VkSemaphore aNextFrameSemaphore, VkFence aNextFrameFence) {
		// Make read index the previous frame that was drawn to.
		ReadIndex = DrawIndex;
		// Generate next draw index.
		DrawIndex = ((DrawIndex == (Image.size() - 1)) ? 0 : (DrawIndex + 1));
		// Return VK_SUCCESS if not system_window.
		return VK_SUCCESS;
	}

	std::vector<command_batch> framechain::predraw() {
		return { PredrawFrameOperation[DrawIndex] };
	}

	std::vector<command_batch> framechain::postdraw() {
		return { PostdrawFrameOperation[DrawIndex] };
	}

}

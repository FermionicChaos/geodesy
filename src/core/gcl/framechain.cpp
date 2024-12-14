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

		// Setup next image semaphore.
		std::vector<VkSemaphore> NextImageSemaphoreList = aContext->create_semaphore(Image.size(), 0);
		for (size_t i = 0; i < NextImageSemaphoreList.size(); i++) {
			this->NextImageSemaphore.push(NextImageSemaphoreList[i]);
		}
	}

	framechain::~framechain() {
		// Destroy Semaphores.
		while (!this->NextImageSemaphore.empty()) {
			VkSemaphore Semaphore = this->NextImageSemaphore.front();
			this->NextImageSemaphore.pop();
			this->Context->destroy_semaphore(Semaphore);
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

	command_batch framechain::next_frame() {
		ReadIndex = DrawIndex;
		DrawIndex = ((DrawIndex == (Image.size() - 1)) ? 0 : (DrawIndex + 1));
		return PredrawFrameOperation[DrawIndex];
	}

	std::vector<command_batch> framechain::present_frame() {
		return { PostdrawFrameOperation[DrawIndex]};
	}

}

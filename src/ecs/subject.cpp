#include <geodesy/ecs/stage.h>
#include <geodesy/ecs/subject.h>

namespace geodesy::ecs {

	using namespace core;

	subject::subject(
		std::shared_ptr<core::gcl::context> aContext, 
		stage* aStage, 
		std::string aName, 
		core::math::vec<uint, 3> 
		aFrameResolution, 
		double aFrameRate, 
		uint32_t aFrameCount, 
		uint32_t aAttachmentCount, 
		core::math::vec<float, 3> aPosition, 
		core::math::vec<float, 2> aDirection
	) : object(aContext, aStage, aName, aPosition, aDirection) {

		this->CommandPool = std::make_shared<gcl::command_pool>(aContext, gcl::device::operation::GRAPHICS_AND_COMPUTE);
		// this->Timer;
	}

	subject::~subject() {

	}

	bool subject::is_subject() {
		return true;
	}

	bool subject::ready_to_render() {
		return this->Framechain->Timer.check();
	}

	VkResult subject::next_frame_now() {
		VkResult Result = VK_SUCCESS;
		
		VkFence Fence = this->Context->create_fence();
		// Acquire next image from swapchain.
		Result = this->next_frame(VK_NULL_HANDLE, Fence);

		Context->wait_and_reset(Fence);

		Context->destroy_fence(Fence);

		return Result;
	}

	std::map<std::string, std::shared_ptr<core::gcl::image>> subject::current_frame() {
		return this->Framechain->Image[this->Framechain->DrawIndex];
	}

	VkResult subject::present_frame_now() {
		VkResult Result = VK_SUCCESS;

		VkPresentInfoKHR PresentInfo = this->present_frame();

		Result = Context->present({ PresentInfo });

		return Result;
	}


	// uint32_t subject::descriptor_set_count() {
	// 	uint32_t DescriptorSetCount = 0;
	// 	for (size_t i = 0; this->Pipeline.size(); i++) {
	// 		DescriptorSetCount += this->Pipeline[i]->DescriptorSetLayout.size();
	// 	}
	// 	return DescriptorSetCount;
	// }

	// std::vector<VkDescriptorPoolSize> subject::descriptor_pool_sizes() {
	// 	// Calculate pool sizes based on pipeline descriptor set layout binding info.
	// 	std::map<VkDescriptorType, uint32_t> DescriptorTypeCount;
	// 	for (size_t i = 0; i < this->Pipeline.size(); i++) {
	// 		switch(this->Pipeline[i]->BindPoint) {
	// 			case VK_PIPELINE_BIND_POINT_GRAPHICS:
	// 				{
	// 					gcl::pipeline::rasterizer* R = this->Pipeline[i]->Rasterizer;
	// 					for (size_t j = 0; j < R->DescriptorSetLayoutBinding.size(); j++) {
	// 						for (size_t k = 0; k < R->DescriptorSetLayoutBinding[j].size(); k++) {
	// 							VkDescriptorSetLayoutBinding DSLB = R->DescriptorSetLayoutBinding[j][k];
	// 							if (DescriptorTypeCount.count(DSLB.descriptorType) == 0) {
	// 								DescriptorTypeCount[DSLB.descriptorType] = 0;
	// 							}
	// 							DescriptorTypeCount[DSLB.descriptorType] += DSLB.descriptorCount;
	// 						}
	// 					}
	// 				}
	// 				break;
	// 			case VK_PIPELINE_BIND_POINT_COMPUTE:
	// 				break;
	// 			case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
	// 				break;
	// 		}
	// 	}

	// 	// Convert to pool size to vector data structure.
	// 	std::vector<VkDescriptorPoolSize> PoolSize;
	// 	for (auto& [Type, Count] : DescriptorTypeCount) {
	// 		VkDescriptorPoolSize DPS{};
	// 		DPS.type = Type;
	// 		DPS.descriptorCount = Count;
	// 		PoolSize.push_back(DPS);
	// 	}

	// 	return PoolSize;
	// }

	// void subject::begin(VkCommandBuffer aCommandBuffer, uint32_t aFrameIndex, VkSubpassContents aSubpassContents) {
	// 	VkRenderPassBeginInfo RPBI{};
	// 	RPBI.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	// 	RPBI.pNext				= NULL;
	// 	RPBI.renderPass			= this->RenderPass;
	// 	RPBI.framebuffer		= this->Frame[aFrameIndex].Buffer;
	// 	RPBI.renderArea			= this->RenderArea;
	// 	RPBI.clearValueCount	= 0;
	// 	RPBI.pClearValues		= NULL;
	// 	vkCmdBeginRenderPass(aCommandBuffer, &RPBI, aSubpassContents);
	// }
	
	// void subject::next(VkCommandBuffer aCommandBuffer, VkSubpassContents aSubpassContents) {
	// 	vkCmdNextSubpass(aCommandBuffer, aSubpassContents);
	// }

	// void subject::end(VkCommandBuffer aCommandBuffer) {
	// 	vkCmdEndRenderPass(aCommandBuffer);
	// }

	std::vector<std::vector<core::gfx::draw_call>> subject::default_renderer(object* aObject) {
		std::vector<std::vector<core::gfx::draw_call>> DefaultRenderer;
		return DefaultRenderer;
	}

	VkResult subject::next_frame(VkSemaphore aSemaphore, VkFence aFence) {
		this->Framechain->ReadIndex = this->Framechain->DrawIndex;
		this->Framechain->DrawIndex = ((this->Framechain->DrawIndex == (Framechain->Image.size() - 1)) ? 0 : (this->Framechain->DrawIndex + 1));
		return VK_SUCCESS;
	}

	std::vector<VkSubmitInfo> subject::render(stage* aStage) {
		std::vector<VkSubmitInfo> StageRender;

		// This is the default render methods for a subject, it does
		// not sort objects by render order, and does not use semaphores
		for (std::shared_ptr<object> Obj : aStage->Object) {
			// Gather draw calls on object.
			std::vector<gfx::draw_call> DrawCall = Obj->draw(this);
			// Seperate command buffers.
			std::vector<VkCommandBuffer> ExtractedCommandBuffer(DrawCall.size());
			for (std::size_t i = 0; i < DrawCall.size(); i++) {
				ExtractedCommandBuffer[i] = DrawCall[i].DrawCommand;
			}
			// Append to list.
			this->Framechain->DrawCommand[this->Framechain->DrawIndex].insert(this->Framechain->DrawCommand[this->Framechain->DrawIndex].end(), ExtractedCommandBuffer.begin(), ExtractedCommandBuffer.end());
		}

		// No draw calls, return empty list.
		if (this->Framechain->DrawCommand[this->Framechain->DrawIndex].size() == 0) {
			return StageRender;
		}

		// Fill out submit info structure.
		VkSubmitInfo Submission{};
		Submission.sType 					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		Submission.pNext 					= NULL;
		Submission.waitSemaphoreCount 		= 0;
		Submission.pWaitSemaphores 			= NULL;
		Submission.pWaitDstStageMask 		= NULL;
		Submission.commandBufferCount 		= this->Framechain->DrawCommand[this->Framechain->DrawIndex].size();
		Submission.pCommandBuffers 			= this->Framechain->DrawCommand[this->Framechain->DrawIndex].data();
		Submission.signalSemaphoreCount 	= 0;
		Submission.pSignalSemaphores 		= NULL;

		// Append to list.
		StageRender.push_back(Submission);

		return StageRender;
	}

	VkPresentInfoKHR subject::present_frame(const std::vector<VkSemaphore>& aWaitSemaphore) {
		VkPresentInfoKHR PresentInfo{};
		PresentInfo.sType 					= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		PresentInfo.pNext 					= NULL;
		PresentInfo.waitSemaphoreCount 		= 0;
		PresentInfo.pWaitSemaphores 		= NULL;
		PresentInfo.swapchainCount 			= 0;
		PresentInfo.pSwapchains 			= NULL;
		PresentInfo.pImageIndices 			= NULL;
		PresentInfo.pResults 				= NULL;
		return PresentInfo;
	}

}
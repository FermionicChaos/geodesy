#pragma once
#ifndef GEODESY_CORE_SUBJECT_H
#define GEODESY_CORE_SUBJECT_H

#include "object.h"

namespace geodesy::ecs {
	
	class subject : public object {
	public:

		struct render_info {
			std::vector<VkSubmitInfo> 					SubmitInfo;
			std::vector<VkPresentInfoKHR> 				PresentInfo;
		};

		std::shared_ptr<core::gcl::command_pool>				CommandPool;
		std::shared_ptr<core::gcl::framechain> 					Framechain;
		std::shared_ptr<core::gcl::pipeline> 					Pipeline;

		subject(std::shared_ptr<core::gcl::context> aContext, stage* aStage, std::string aName, core::math::vec<uint, 3> aFrameResolution, double aFrameRate, uint32_t aFrameCount, uint32_t aAttachmentCount);
		~subject();

		virtual bool is_subject() override;

		bool ready_to_render();
		// uint32_t descriptor_set_count();
		// std::vector<VkDescriptorPoolSize> descriptor_pool_sizes();
		// void begin(VkCommandBuffer aCommandBuffer, uint32_t aFrameIndex, VkSubpassContents aSubpassContents = VK_SUBPASS_CONTENTS_INLINE);
		// void next(VkCommandBuffer aCommandBuffer, VkSubpassContents aSubpassContents = VK_SUBPASS_CONTENTS_INLINE);
		// void end(VkCommandBuffer aCommandBuffer);

		virtual std::shared_ptr<core::gfx::renderer> make_default_renderer(object* aObject);
		virtual VkResult next_frame(VkSemaphore aSemaphore = VK_NULL_HANDLE, VkFence aFence = VK_NULL_HANDLE);
		virtual std::vector<VkSubmitInfo> render(stage* aStage);
		virtual VkPresentInfoKHR present_frame(const std::vector<VkSemaphore>& aWaitSemaphore = {});

	};

	

}

#endif // GEODESY_CORE_SUBJECT_H
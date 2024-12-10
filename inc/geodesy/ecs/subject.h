#pragma once
#ifndef GEODESY_CORE_SUBJECT_H
#define GEODESY_CORE_SUBJECT_H

#include "../config.h"

#include "../core/io.h"
#include "../core/math.h"
#include "../core/util.h"
#include "../core/lgc.h"
#include "../core/phys.h"
#include "../core/hid.h"
#include "../core/gcl.h"
#include "../core/gfx.h"
#include "../core/sfx.h"

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

		subject(
			std::shared_ptr<core::gcl::context> aContext, 
			stage* aStage, 
			std::string aName, 
			core::math::vec<uint, 3> 
			aFrameResolution, 
			double aFrameRate, 
			uint32_t aFrameCount, 
			uint32_t aAttachmentCount, 
			core::math::vec<float, 3> aPosition = { 0.0f, 0.0f, 0.0f }, 
			core::math::vec<float, 2> aDirection = { 90.0f, 90.0f }
		);
		~subject();

		virtual bool is_subject() override;

		bool ready_to_render();
		VkResult next_frame_now();
		std::map<std::string, std::shared_ptr<core::gcl::image>> read_frame();
		std::map<std::string, std::shared_ptr<core::gcl::image>> draw_frame();
		VkResult present_frame_now();

		virtual std::vector<std::vector<core::gfx::draw_call>> default_renderer(object* aObject);
		virtual VkResult next_frame(VkSemaphore aSemaphore = VK_NULL_HANDLE, VkFence aFence = VK_NULL_HANDLE);
		virtual std::vector<VkSubmitInfo> render(stage* aStage);
		virtual VkPresentInfoKHR present_frame(const std::vector<VkSemaphore>& aWaitSemaphore = {});

	};

	

}

#endif // GEODESY_CORE_SUBJECT_H
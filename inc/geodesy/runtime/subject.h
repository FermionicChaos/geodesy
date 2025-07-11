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
#include "../core/gpu.h"
#include "../core/gfx.h"
#include "../core/sfx.h"

#include "object.h"

namespace geodesy::runtime {
	
	class subject : public object {
	public:

		class framechain {
		public:
		
			core::math::vec<uint, 3>													Resolution;
			uint32_t																	ReadIndex;
			uint32_t																	DrawIndex;
			double																		FrameRate;
			core::lgc::timer 															Timer;
			std::shared_ptr<core::gpu::context> 										Context;
			std::vector<std::map<std::string, std::shared_ptr<core::gpu::image>>> 		Image;
			std::vector<core::gpu::command_batch> 										PredrawFrameOperation;
			std::vector<core::gpu::command_batch> 										PostdrawFrameOperation;
		
			framechain(std::shared_ptr<core::gpu::context> aContext, double aFrameRate, uint32_t aFrameCount);
			~framechain();
		
			std::map<std::string, std::shared_ptr<core::gpu::image>> read_frame();
			std::map<std::string, std::shared_ptr<core::gpu::image>> draw_frame();
			bool ready_to_render();
			VkResult next_frame_now();
			VkResult present_frame_now();
		
			// This function is special because it presents, and acquires next frame.
			virtual VkResult next_frame(VkSemaphore aPresentFrameSemaphore = VK_NULL_HANDLE, VkSemaphore aNextFrameSemaphore = VK_NULL_HANDLE, VkFence aNextFrameFence = VK_NULL_HANDLE);
			virtual std::vector<core::gpu::command_batch> predraw();
			virtual std::vector<core::gpu::command_batch> postdraw();
		
		};

		struct uniform_data {
			alignas(16) core::math::vec<float, 3> 		Position;
			alignas(16) core::math::mat<float, 4, 4> 	Rotation;
			alignas(16) core::math::mat<float, 4, 4> 	Projection;
			alignas(16) core::math::mat<float, 4, 4> 	PRT;
			uniform_data(
				core::math::vec<float, 3> 		aPosition, 
				core::math::vec<float, 2> 		aDirection,
				core::math::vec<float, 3> 		aScale,
				float 							aNear,
				float 							aFar
			);
			uniform_data(
				core::math::vec<float, 3> 		aPosition, 
				core::math::vec<float, 2> 		aDirection,
				float 							aFOV,
				core::math::vec<uint, 3> 		aResolution,
				float 							aNear,
				float 							aFar
			);
		};

		struct creator : object::creator {
			core::math::vec<uint, 3> 		Resolution;
			uint 							FrameCount;
			float 							FrameRate;
			int 							ImageUsage;
			creator();
		};
				
		std::shared_ptr<framechain> 											Framechain;
		std::vector<std::vector<std::shared_ptr<core::gpu::framebuffer>>> 		Framebuffer;
		std::vector<std::shared_ptr<core::gpu::pipeline>> 						Pipeline;
		std::shared_ptr<core::gpu::buffer> 										SubjectUniformBuffer;
		std::shared_ptr<core::gpu::command_pool>								CommandPool;
		std::shared_ptr<core::gpu::semaphore_pool> 								SemaphorePool;
		std::vector<core::gpu::command_batch>									RenderingOperations;
		VkSemaphore 															NextFrameSemaphore;
		VkSemaphore 															PresentFrameSemaphore;

		subject(std::shared_ptr<core::gpu::context> aContext, stage* aStage, creator* aSubjectCreator);
		~subject();

		virtual bool is_subject() override;
		virtual std::shared_ptr<renderer> default_renderer(object* aObject);
		virtual core::gpu::submission_batch render(stage* aStage);

	};

	

}

#endif // GEODESY_CORE_SUBJECT_H
#pragma once
#ifndef GEODESY_CORE_GPU_FRAMECHAIN_H
#define GEODESY_CORE_GPU_FRAMECHAIN_H

#include "../../config.h"
#include "../math.h"
#include "../lgc/timer.h"
#include "config.h"
#include "semaphore_pool.h"
#include "command_batch.h"
#include "image.h"

namespace geodesy::core::gpu {

	class framechain {
	public:

		math::vec<uint, 3>												Resolution;
		uint32_t														ReadIndex;
		uint32_t														DrawIndex;
		double															FrameRate;
		core::lgc::timer 												Timer;
		std::shared_ptr<context> 										Context;
		std::vector<std::map<std::string, std::shared_ptr<image>>> 		Image;
		std::vector<command_batch> 										PredrawFrameOperation;
		std::vector<command_batch> 										PostdrawFrameOperation;

		framechain(std::shared_ptr<context> aContext, double aFrameRate, uint32_t aFrameCount);
		~framechain();

		std::map<std::string, std::shared_ptr<image>> read_frame();
		std::map<std::string, std::shared_ptr<image>> draw_frame();
		bool ready_to_render();
		VkResult next_frame_now();
		VkResult present_frame_now();

		// This function is special because it presents, and acquires next frame.
		virtual VkResult next_frame(VkSemaphore aPresentFrameSemaphore = VK_NULL_HANDLE, VkSemaphore aNextFrameSemaphore = VK_NULL_HANDLE, VkFence aNextFrameFence = VK_NULL_HANDLE);
		virtual std::vector<command_batch> predraw();
		virtual std::vector<command_batch> postdraw();

	}; 	

}

#endif // !GEODESY_CORE_GPU_FRAMECHAIN_H
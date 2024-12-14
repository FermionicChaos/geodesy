#pragma once
#ifndef GEODESY_CORE_GCL_FRAMECHAIN_H
#define GEODESY_CORE_GCL_FRAMECHAIN_H

#include "../../config.h"
#include "../math.h"
#include "../lgc/timer.h"
#include "config.h"
#include "semaphore_pool.h"
#include "command_batch.h"
#include "image.h"

namespace geodesy::core::gcl {

	class framechain {
	public:

		math::vec<uint, 3>												Resolution;
		uint32_t														ReadIndex;
		uint32_t														DrawIndex;
		double															FrameRate;
		core::lgc::timer 												Timer;
		std::shared_ptr<context> 										Context;
		std::vector<std::map<std::string, std::shared_ptr<image>>> 		Image;
		std::queue<VkSemaphore> 										NextImageSemaphore;
		std::vector<command_batch> 										PredrawFrameOperation;
		std::vector<command_batch> 										PostdrawFrameOperation;

		framechain(std::shared_ptr<context> aContext, double aFrameRate, uint32_t aFrameCount);
		~framechain();

		std::map<std::string, std::shared_ptr<image>> read_frame();
		std::map<std::string, std::shared_ptr<image>> draw_frame();
		bool ready_to_render();
		VkResult next_frame_now();
		VkResult present_frame_now();

		virtual command_batch next_frame();
		virtual std::vector<command_batch> present_frame();

	}; 	

}

#endif // !GEODESY_CORE_GCL_FRAMECHAIN_H
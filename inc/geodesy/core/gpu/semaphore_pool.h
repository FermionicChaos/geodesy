#pragma once
#ifndef GEODESY_CORE_GPU_SEMAPHORE_POOL_H
#define GEODESY_CORE_GPU_SEMAPHORE_POOL_H

#include "../../config.h"
#include "config.h"

namespace geodesy::core::gpu {

	// This class is used to manage in bulk large ammounts of semaphores.

	class semaphore_pool {
	public:

		std::shared_ptr<context> 			Context;
		std::vector<VkSemaphore> 			Total;
		std::queue<VkSemaphore> 			Available;
		std::unordered_set<VkSemaphore> 	InUse;

		semaphore_pool(std::shared_ptr<context> aContext, size_t aSemaphoreCount);
		~semaphore_pool();

		VkSemaphore acquire();
		void release(VkSemaphore aSemaphore);
		void reset();
		
	};
}

#endif // !GEODESY_CORE_GPU_SEMAPHORE_POOL_H
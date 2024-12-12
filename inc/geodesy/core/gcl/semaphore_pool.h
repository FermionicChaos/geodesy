#pragma once
#ifndef GEODESY_CORE_GCL_SEMAPHORE_POOL_H
#define GEODESY_CORE_GCL_SEMAPHORE_POOL_H

#include "../../config.h"
#include "config.h"

namespace geodesy::core::gcl {

	// This class is used to manage in bulk large ammounts of semaphores.

	class semaphore_pool {
	public:

		std::shared_ptr<context> 			Context;
		std::vector<VkSemaphore> 			SemaphoreList;
		std::unordered_set<VkSemaphore> 	SemaphoreInUse;

		semaphore_pool(std::shared_ptr<context> aContext, size_t aSemaphoreCount);
		~semaphore_pool();

		VkSemaphore acquire();
		void release(VkSemaphore aSemaphore);
		void reset();
		
	};
}

#endif // !GEODESY_CORE_GCL_SEMAPHORE_POOL_H
#include <geodesy/core/gcl/semaphore_pool.h>

#include <geodesy/core/gcl/context.h>

namespace geodesy::core::gcl {

	semaphore_pool::semaphore_pool(std::shared_ptr<context> aContext, size_t aSemaphoreCount) {
		this->Context = aContext;
		std::vector<VkSemaphore> SemaphoreList = aContext->create_semaphore(aSemaphoreCount, 0);
	}

	semaphore_pool::~semaphore_pool() {
		this->Context->destroy_semaphore(this->SemaphoreList);
	}

	VkSemaphore semaphore_pool::aquire() {
		VkSemaphore Semaphore = this->SemaphoreList.back();
		this->SemaphoreList.pop_back();
		this->SemaphoreInUse.insert(Semaphore);
		return Semaphore;
	}

	void semaphore_pool::release(VkSemaphore aSemaphore) {
		// If does not belong to this pool, ignore. Does not own it.
		if (this->SemaphoreInUse.count(aSemaphore) == 0) return;
		// Add semaphore back to pool.
		this->SemaphoreList.push_back(aSemaphore);
		// Remove from in use list.
		this->SemaphoreInUse.erase(aSemaphore);
	}


}

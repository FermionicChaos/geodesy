#include <geodesy/core/gpu/semaphore_pool.h>

#include <geodesy/core/gpu/context.h>

namespace geodesy::core::gpu {

	semaphore_pool::semaphore_pool(std::shared_ptr<context> aContext, size_t aSemaphoreCount) {
		this->Context = aContext;
		this->Total = aContext->create_semaphore(aSemaphoreCount, 0);
		for (size_t i = 0; i < aSemaphoreCount; i++) {
			this->Available.push(this->Total[i]);
		}
	}

	semaphore_pool::~semaphore_pool() {
		this->Context->destroy_semaphore(this->Total);
	}

	VkSemaphore semaphore_pool::acquire() {
		VkSemaphore Semaphore = this->Available.front();
		this->Available.pop();
		this->InUse.insert(Semaphore);
		return Semaphore;
	}

	void semaphore_pool::release(VkSemaphore aSemaphore) {
		// If does not belong to this pool, ignore. Does not own it.
		if (this->InUse.count(aSemaphore) == 0) return;
		// Add semaphore back to pool.
		this->Available.push(aSemaphore);
		// Remove from in use list.
		this->InUse.erase(aSemaphore);
	}

	void semaphore_pool::reset() {
		for (VkSemaphore Semaphore : this->InUse) {
			this->Available.push(Semaphore);
		}
		this->InUse.clear();
	}

}

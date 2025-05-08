#include <geodesy/runtime/stage.h>

// #define ENABLE_MULTITHREADED_PROCESSING

#ifdef ENABLE_MULTITHREADED_PROCESSING
#include <omp.h>
#endif // ENABLE_MULTITHREADED_PROCESSING

#include <iostream>

namespace geodesy::runtime {

	using namespace core;

	stage::stage(std::shared_ptr<gpu::context> aContext, std::string aName) {
		this->Name		= aName;
		this->Context	= aContext;
	}

	stage::~stage() {

	}

	// Does Nothing by default.
	gpu::submission_batch stage::update(double aDeltaTime) {
		gpu::submission_batch StageUpdateInfo;
#ifdef ENABLE_MULTITHREADED_PROCESSING

		/*
		// This list contains the pairs that have been detected to be in collision on broad phase metrics.
		std::vector<std::pair<object*, object*>> BroadPhaseCollisionPair;
		
		// This list contains the pairs that have been detected to be in collision on narrow phase metrics.
		std::vector<std::pair<object*, object*>> NarrowPhaseCollisionPair;
		
		// TODO: Broad phase collision detection will use bounding spheres based on the objects farthest vertex.
		// Yes, I know this is n2, suck my dick. I'll change it later.
		// for (size_t i = 0; i < this->Object.size(); i++) {
		// 	for (size_t j = i + 1; j < this->Object.size(); j++) {
		// 	}
		// }
		
		// TODO: Narrow phase collision detection will use the actual geometry of the objects.
		
		// TODO: This section of the code will evaluate collision responses between pairs.
		*/

		// After collision has been completed, and response forces determined, update objects accordingly.

		// Determine workload for each thread.
		std::vector<workload> DistributedThreadWorkload = stage::determine_thread_workload(this->Object.size(), omp_get_max_threads());
		#pragma omp parallel
		{
			size_t ThreadIndex = omp_get_thread_num();
			size_t ObjectStartIndex = DistributedThreadWorkload[ThreadIndex].Start;
			size_t ObjectEndIndex = DistributedThreadWorkload[ThreadIndex].Start + DistributedThreadWorkload[ThreadIndex].Count;
			for (size_t i = ObjectStartIndex; i < ObjectEndIndex; i++) {
				Object[i]->update(aDeltaTime);
			}
		}
#else
		for (auto& Obj : Object) {
			Obj->update(aDeltaTime);
		}
#endif // ENABLE_MULTITHREADED_PROCESSING

		return StageUpdateInfo;
	}

	gpu::submission_batch stage::render() {
		gpu::submission_batch RenderInfo;

		// Generate list of render targets in this stage.
		std::vector<subject*> RenderTargetList = stage::purify_by_subject(this->Object);

		// For each render target, render the stage.
		for (subject* RenderTarget : RenderTargetList) {

			// Check if Render Target is ready to render.
			if (!RenderTarget->Framechain->ready_to_render()) continue;

			// Clear out previous rendering operations.
			RenderTarget->RenderingOperations = std::vector<gpu::command_batch>();

			// Reset semaphore pool.
			RenderTarget->SemaphorePool->reset();

			// Gather render operations per target.
			RenderInfo += RenderTarget->render(this);
		}

		return RenderInfo;
	}

	std::vector<subject*> stage::purify_by_subject(const std::vector<std::shared_ptr<object>>& aObjectList) {
		std::vector<subject*> SubjectList;
		for (std::shared_ptr<object> Obj : aObjectList) {
			if (Obj->is_subject()) {
				SubjectList.push_back(static_cast<subject*>(Obj.get()));
			}
		}
		return SubjectList;
	}

	// TODO: This could be implemented if object update times were measured.
	// * This function distrbutes the work evenly to each thread.
	std::vector<stage::workload> stage::determine_thread_workload(size_t aElementCount, size_t aThreadCount) {
		std::vector<workload> Workload(aThreadCount);
		size_t RemainderCount = aElementCount % aThreadCount;
		size_t ElementsPerThread = (aElementCount - RemainderCount) / aThreadCount;
		size_t Offset = 0;
		for (size_t i = 0; i < Workload.size(); i++) {
			Workload[i].Start = Offset;
			Workload[i].Count = ElementsPerThread;
			if (i < RemainderCount) {
				Workload[i].Count += 1;
			}
			Offset += Workload[i].Count;
		}
		return Workload;
	}

}

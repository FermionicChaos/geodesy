#include <geodesy/runtime/stage.h>

// Why tf is single threaded better?
// #define ENABLE_MULTITHREADED_PROCESSING

#ifdef ENABLE_MULTITHREADED_PROCESSING
#include <omp.h>
#endif // ENABLE_MULTITHREADED_PROCESSING

#include <iostream>

namespace geodesy::runtime {

	using namespace core;

	stage::stage(std::shared_ptr<gpu::context> aContext, std::string aName) {
		this->Name		= aName;
		this->Time		= 0.0;
		this->Context	= aContext;
	}

	stage::~stage() {

	}

	// Does Nothing by default.
	gpu::submission_batch stage::update(double aDeltaTime) {
		gpu::submission_batch StageUpdateInfo;

		this->Time += aDeltaTime;

		// Acquire all notes in the stage.
		size_t NodeCountTotal = 0;
		for (auto& Obj : this->Object) {
			NodeCountTotal += Obj->LinearizedNodeTree.size();
		}

		if (NodeCountTotal != this->NodeCache.size()) {
			this->NodeCache.resize(NodeCountTotal);
	
			size_t NodeIndex = 0;
			for (size_t i = 0; i < this->Object.size(); i++) {
				for (size_t j = 0; j < this->Object[i]->LinearizedNodeTree.size(); j++) {
					this->NodeCache[NodeIndex++] = this->Object[i]->LinearizedNodeTree[j];
				}
			}
		}

		// This list contains the pairs that have been detected to be in collision on broad phase metrics.
		// std::vector<std::pair<object*, object*>> BroadPhaseCollisionPair;
		
		// This list contains the pairs that have been detected to be in collision on narrow phase metrics.
		// std::vector<std::pair<object*, object*>> NarrowPhaseCollisionPair;
		
		// TODO: Broad phase collision detection will use bounding spheres based on the objects farthest vertex.
		// Yes, I know this is n2, suck my dick. I'll change it later.
				
		// TODO: Narrow phase collision detection will use the actual geometry of the objects.
				
		// TODO: This section of the code will evaluate collision responses between pairs.
		
		// After collision has been completed, and response forces determined, update objects accordingly.
			
#ifdef ENABLE_MULTITHREADED_PROCESSING
		#pragma omp parallel for
#endif // ENABLE_MULTITHREADED_PROCESSING
		for (std::ptrdiff_t i = 0; i < this->NodeCache.size(); i++) {
			object* Object = static_cast<object*>(this->NodeCache[i]->Root);
			this->NodeCache[i]->update(aDeltaTime, this->Time, Object->AnimationWeights, Object->Model->Animation);
		}

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

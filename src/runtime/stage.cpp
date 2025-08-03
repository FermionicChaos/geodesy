#include <geodesy/runtime/stage.h>

// Built-in object includes
#include <geodesy/bltn/obj/camera3d.h>
#include <geodesy/bltn/obj/cameravr.h>
#include <geodesy/bltn/obj/window.h>
#include <geodesy/bltn/obj/subject_window.h>
#include <geodesy/bltn/obj/system_window.h>

#include <geodesy/runtime/app.h>

// Why tf is single threaded better?
// #define ENABLE_MULTITHREADED_PROCESSING

#ifdef ENABLE_MULTITHREADED_PROCESSING
#include <omp.h>
#endif // ENABLE_MULTITHREADED_PROCESSING

#include <iostream>

namespace geodesy::runtime {

	using namespace core;
	using namespace bltn::obj;

	stage::creator::creator() {
		this->Name = "";
		this->RTTIID = stage::rttiid;
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

	stage::stage(std::shared_ptr<core::gpu::context> aContext, creator* aCreator) {
		this->Name		= aCreator->Name;
		this->Time		= 0.0;
		this->Context	= aContext;

		// Create Stage Objects.
		this->Object = this->build_objects(aContext, aCreator->ObjectCreationList);

		// Linearize and cache all nodes in the stage.
		this->build_node_cache();

		// Setup global transforms for all nodes in the stage.
		for (std::ptrdiff_t i = 0; i < this->NodeCache.size(); i++) {
			// Recursively generate global transforms for all nodes.
			this->NodeCache[i]->GlobalTransform = this->NodeCache[i]->transform();
		}

		gpu::buffer::create_info MaterialBufferCreateInfo;
		MaterialBufferCreateInfo.Memory = gpu::device::memory::HOST_VISIBLE | gpu::device::memory::HOST_COHERENT;
		MaterialBufferCreateInfo.Usage = gpu::buffer::usage::UNIFORM | gpu::buffer::usage::TRANSFER_SRC | gpu::buffer::usage::TRANSFER_DST;

		gpu::buffer::create_info LightBufferCreateInfo;
		LightBufferCreateInfo.Memory = gpu::device::memory::HOST_VISIBLE | gpu::device::memory::HOST_COHERENT;
		LightBufferCreateInfo.Usage = gpu::buffer::usage::UNIFORM | gpu::buffer::usage::TRANSFER_SRC | gpu::buffer::usage::TRANSFER_DST;

		// Create Light Storage Buffer.
		this->MaterialUniformBuffer = this->Context->create_buffer(MaterialBufferCreateInfo, sizeof(material_uniform_data));
		this->MaterialUniformBuffer->map_memory(0, sizeof(material_uniform_data));
		this->LightUniformBuffer = this->Context->create_buffer(LightBufferCreateInfo, sizeof(light_uniform_data));
		this->LightUniformBuffer->map_memory(0, sizeof(light_uniform_data));

		// Build Global Scene Geometry & Resource References for Ray Tracing.
		this->build_scene_geometry();

	}

	stage::~stage() {

	}

	std::vector<std::shared_ptr<object>> stage::build_objects(std::shared_ptr<core::gpu::context> aContext, std::vector<object::creator*> aCreationList) {
		std::vector<std::shared_ptr<object>> ObjectList;
		for (auto& Creator : aCreationList) {
			std::shared_ptr<object> NewObject = stage::build_object(aContext, this, Creator);
			if (NewObject != nullptr) {
				ObjectList.push_back(NewObject);
			}
		}
		return ObjectList;
	}

	std::shared_ptr<object> stage::build_object(std::shared_ptr<core::gpu::context> aContext, stage* aStage, object::creator* aObjectCreator) {
		switch(aObjectCreator->RTTIID) {
		case object::rttiid: 			return stage::create<object>(aContext, aStage, aObjectCreator);
		// case subject::rttiid: 			return stage::create<subject>(aContext, aStage, aObjectCreator); // Not Creatable
		case camera3d::rttiid: 			return stage::create<camera3d>(aContext, aStage, aObjectCreator);
		case cameravr::rttiid: 			return stage::create<cameravr>(aContext, aStage, aObjectCreator);
		// case window::rttiid: 			return stage::create<window>(aContext, aStage, aObjectCreator); // Not Creatable
		case subject_window::rttiid: 	return stage::create<subject_window>(aContext, aStage, aObjectCreator);
		case system_window::rttiid: 	return stage::create<system_window>(aContext, aStage, aObjectCreator);
		default: 						return nullptr;
		}
	}

	void stage::build_node_cache() {
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
	}

	void stage::build_scene_geometry() {

		// Build stage node cache.
		this->build_node_cache();

		for (std::ptrdiff_t i = 0; i < this->NodeCache.size(); i++) {
			// Recursively generate global transforms for all nodes.
			this->NodeCache[i]->GlobalTransform = this->NodeCache[i]->transform();
		}

		// Build TLAS.
		this->TLAS = geodesy::make<gpu::acceleration_structure>(this->Context, this);

	}

	// Does Nothing by default.
	void stage::update(double aDeltaTime) {
		this->Time += aDeltaTime;

		// Build Node Cache.
		this->build_node_cache();

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
			// Perform all host memory calculations, apply forces and animations
			NodeCache[i]->host_update(aDeltaTime, this->Time);
		}

#ifdef ENABLE_MULTITHREADED_PROCESSING
		#pragma omp parallel for
#endif // ENABLE_MULTITHREADED_PROCESSING
		for (std::ptrdiff_t i = 0; i < this->NodeCache.size(); i++) {
			// Recursively generate global transforms for all nodes.
			this->NodeCache[i]->GlobalTransform = this->NodeCache[i]->transform();
		}

		// This is serialized because the GPU memory is not thread safe.
		for (std::ptrdiff_t i = 0; i < this->NodeCache.size(); i++) {
			// Load Global Transforms into GPU memory for rendering.
			this->NodeCache[i]->device_update();
		}
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

	std::vector<std::shared_ptr<object::draw_call>> stage::ray_trace(subject* aSubject) {
		
		if (this->Renderer.count(aSubject) == 0) {
			// Get Draw Call
			this->Renderer[aSubject] = aSubject->default_ray_tracer(this);
		}

		// Return draw calls for the subject.
		return (*this->Renderer[aSubject])[aSubject->Framechain->DrawIndex];
	}

}

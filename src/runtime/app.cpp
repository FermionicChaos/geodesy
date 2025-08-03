#include <geodesy/engine.h>

// Built-in objects and stages for core geodesy engine.
#include <geodesy/bltn.h>

#include <omp.h>

namespace geodesy::runtime {

	using namespace core;

	using namespace bltn::obj;

	app::app(engine* aEngine, std::string aName, math::vec<uint, 3> aVersion) {
		this->Engine = aEngine;
		this->Name = aName;
		this->Version = aVersion;
		this->TimeStep = 1.0 / 30.0;
		this->Time = 0.0;
		omp_set_num_threads(omp_get_max_threads());
	}

	app::~app() {
		this->Engine->wait_on_device_context();
		for (auto& Stg : this->Stage) {
			for (auto& Obj : Stg->Object) {
				Obj->Renderer.clear();
			}
		}
	}

	std::shared_ptr<stage> app::build_stage(std::shared_ptr<core::gpu::context> aContext, stage::creator* aStageCreator) {
		switch(aStageCreator->RTTIID) {
		case stage::rttiid: 			return app::create<stage>(aContext, aStageCreator);
		default: 						return nullptr;
		}
	}

	void app::init() {
		this->run();
	}

	void app::update(double aDeltaTime) {
		this->Time += aDeltaTime;
		// Iterate through each stage and update host resources.
		for (auto& Stg : this->Stage) {
			// Update entire stage.
			Stg->update(aDeltaTime);
		}
	}

	std::map<std::shared_ptr<gpu::context>, gpu::submission_batch> app::render() {
		std::map<std::shared_ptr<gpu::context>, gpu::submission_batch> RenderOperations;

		for (auto& Stg : this->Stage) {
			// Gather render information from each stage.
			gpu::submission_batch StageRenderOperations = Stg->render();
			if (RenderOperations.count(Stg->Context) == 0) {
				// If the context doesn't exist, create it.
				RenderOperations[Stg->Context] = StageRenderOperations;
			}
			else {
				// If the context does exist, append the operations.
				RenderOperations[Stg->Context] += StageRenderOperations;
			}
		}

		return RenderOperations;
	}

}
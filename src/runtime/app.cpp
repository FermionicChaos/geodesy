#include <geodesy/engine.h>

// Built-in objects and stages for core geodesy engine.
#include <geodesy/bltn.h>

#include <omp.h>
#include <iostream>

namespace geodesy::runtime {

	using namespace core;
	using namespace lgc;

	using namespace bltn::obj;

	app::app(engine* aEngine, std::string aName, math::vec<uint, 3> aVersion) {
		this->Engine = aEngine;
		this->Name = aName;
		this->Version = aVersion;
		this->TimeStep = 1.0 / 30.0;
		this->Time = 0.0;
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

	void app::initialize() {
		// Set max thread count for OpenMP.
		omp_set_num_threads(omp_get_max_threads());
	}

	void app::run() {
		VkResult Result = VK_SUCCESS;

		timer PerformanceTimer(1.0);

		// Start main loop.
		float t = 0.0f;
		while (Engine->ThreadController.cycle(TimeStep)) {
			t += Engine->ThreadController.total_time() * 100.0f;

			double t1 = timer::get_time();

			system_window::poll_input();

			double t2 = timer::get_time();

			// Update host resources.
			Result = Engine->update_host_resources(this);

			double t3 = timer::get_time();

			// Execute gpu workloads.
			Result = Engine->execute_device_operations(this);

			double t4 = timer::get_time();

			if (PerformanceTimer.check()) {
				std::cout << "----- Performance Metrics -----" << std::endl;
				std::cout << "Current Time:\t" << timer::get_time() << " s" << std::endl;
				std::cout << "Time Step:\t" << TimeStep * 1000 << " ms" << std::endl;
				std::cout << "Work Time:\t" << (t4 - t1) * 1000.0 << " ms" << std::endl;
				std::cout << "-Input Time:\t" << (t2 - t1) * 1000.0 << " ms" << std::endl;
				std::cout << "-Update Time:\t" << (t3 - t2) * 1000.0 << " ms" << std::endl;
				std::cout << "-Render Time:\t" << (t4 - t3) * 1000.0 << " ms" << std::endl;
				std::cout << "Halt Time:\t" << Engine->ThreadController.halt_time() * 1000.0 << " ms" << std::endl;
				std::cout << "Total Time:\t" << Engine->ThreadController.total_time() * 1000.0 << " ms" << std::endl << std::endl;
				//std::cout << "Thread Over Time: " << Engine->ThreadController.work_time() - TimeStep << std::endl;
			}

			// if (timer::get_time() > 30.0f) {
			// 	break;
			// }

		}

		lgc::timer::wait(5.0f);
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
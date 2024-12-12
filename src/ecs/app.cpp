#include <geodesy/ecs/app.h>

#include <omp.h>

namespace geodesy::ecs {

	using namespace core;

	app::app(engine* aEngine, std::string aName, math::vec<uint, 3> aVersion) {
		this->Engine = aEngine;
		this->Name = aName;
		this->Version = aVersion;
		this->TimeStep = 1.0 / 30.0;
		this->Time = 0.0;
		omp_set_num_threads(omp_get_max_threads());
	}

	void app::init() {
		this->run();
	}

	std::map<std::shared_ptr<gcl::context>, gcl::submission_batch> app::update(double aDeltaTime) {
		std::map<std::shared_ptr<gcl::context>, gcl::submission_batch> UpdateOperations;
		this->Time += aDeltaTime;
		// Iterate through each stage and update host resources, while acquiring device update operations.
		for (auto& Stg : this->Stage) {
			// Update entire stage.
			gcl::submission_batch StageUpdateOperations = Stg->update(aDeltaTime);
			if (UpdateOperations.count(Stg->Context) == 0) {
				// If the context doesn't exist, create it.
				UpdateOperations[Stg->Context] = StageUpdateOperations;
			}
			else {
				// If the context does exist, append the operations.
				UpdateOperations[Stg->Context] += StageUpdateOperations;
			}
		}

		return UpdateOperations;
	}

	std::map<std::shared_ptr<gcl::context>, gcl::submission_batch> app::render() {
		std::map<std::shared_ptr<gcl::context>, gcl::submission_batch> RenderOperations;

		for (auto& Stg : this->Stage) {
			// Gather render information from each stage.
			gcl::submission_batch StageRenderOperations = Stg->render();
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
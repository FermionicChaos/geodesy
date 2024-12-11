#include <geodesy/ecs/app.h>

#include <omp.h>

namespace geodesy::ecs {

	using namespace core;

	static std::vector<VkSubmitInfo> purify_execution_vector(const std::vector<VkSubmitInfo>& aInput) {
		size_t OutputCount = 0;
		// Determine number of VkSubmitInfos that have non-zero command buffer count.
		for (size_t i = 0; i < aInput.size(); i++) {
			if (aInput[i].commandBufferCount > 0) {
				OutputCount++;
			}
		}

		// Create Output vector with only non-zero command buffer count VkSubmitInfos.
		std::vector<VkSubmitInfo> Output(OutputCount);
		size_t OutputOffset = 0;

		// Iterate through input vector second time, copy non-zero VkSubmitInfos to Output.
		for (size_t i = 0; i < aInput.size(); i++) {
			if (aInput[i].commandBufferCount > 0) {
				Output[OutputOffset] = aInput[i];
				OutputOffset++;
			}
		}

		return Output;
	}

	static std::vector<VkPresentInfoKHR> purify_presentation_vector(const std::vector<VkPresentInfoKHR>& aInput) {
		size_t OutputCount = 0;
		// Determine number of VkSubmitInfos that have non-zero command buffer count.
		for (size_t i = 0; i < aInput.size(); i++) {
			if (aInput[i].swapchainCount > 0) {
				OutputCount++;
			}
		}

		// Create Output vector with only non-zero command buffer count VkSubmitInfos.
		std::vector<VkPresentInfoKHR> Output(OutputCount);
		size_t OutputOffset = 0;

		// Iterate through input vector second time, copy non-zero VkSubmitInfos to Output.
		for (size_t i = 0; i < aInput.size(); i++) {
			if (aInput[i].swapchainCount > 0) {
				Output[OutputOffset] = aInput[i];
				OutputOffset++;
			}
		}

		return Output;
	}

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

	std::map<std::shared_ptr<gcl::context>, object::update_info> app::update(double aDeltaTime) {
		std::map<std::shared_ptr<gcl::context>, object::update_info> UpdateOperations;
		this->Time += aDeltaTime;
		// Iterate through each stage and update host resources, while acquiring device update operations.
		for (auto& Stg : this->Stage) {
			// Update entire stage.
			object::update_info UpdateInfo = Stg->update(aDeltaTime);
			// Initialize element in the map.
			UpdateOperations[Stg->Context] = object::update_info();
			// Remove empty elements.
			UpdateOperations[Stg->Context].TransferOperations = purify_execution_vector(UpdateInfo.TransferOperations);
			UpdateOperations[Stg->Context].ComputeOperations = purify_execution_vector(UpdateInfo.ComputeOperations);
		}

		return UpdateOperations;
	}

	std::map<std::shared_ptr<gcl::context>, subject::render_info> app::render() {
		std::map<std::shared_ptr<gcl::context>, subject::render_info> RenderOperations;

		for (auto& Stg : this->Stage) {
			// Gather render information from each stage.
			//RenderOperations[Stg->Context] += Stg->render();
			subject::render_info RenderInfo = Stg->render();
			// Purify empty elements.
			RenderInfo.SubmitInfo = purify_execution_vector(RenderInfo.SubmitInfo);
			RenderInfo.PresentInfo = purify_presentation_vector(RenderInfo.PresentInfo);
			// Check if Context key exists in map.
			if (RenderOperations.count(Stg->Context) == 0) {
				// If no key exists, simply add.
				RenderOperations[Stg->Context] = RenderInfo;
			} 
			else {
				// If element exists, append new Submissions/Presentations.
				RenderOperations[Stg->Context].SubmitInfo.insert(RenderOperations[Stg->Context].SubmitInfo.end(), RenderInfo.SubmitInfo.begin(), RenderInfo.SubmitInfo.end());
				RenderOperations[Stg->Context].PresentInfo.insert(RenderOperations[Stg->Context].PresentInfo.end(), RenderInfo.PresentInfo.begin(), RenderInfo.PresentInfo.end());
			}
		}

		return RenderOperations;
	}

}
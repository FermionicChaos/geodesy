#pragma once 
#ifndef GEODESY_CORE_GCL_SUBMISSION_BATCH_H
#define GEODESY_CORE_GCL_SUBMISSION_BATCH_H

#include "../../config.h"
#include "config.h"
#include "command_batch.h"

namespace geodesy::core::gcl {

	class submission_batch {
	public:

		std::vector<VkSubmitInfo> 		SubmitInfo;

		submission_batch& operator+=(const VkSubmitInfo& aSubmitInfo);
		submission_batch& operator+=(const std::vector<VkSubmitInfo>& aSubmitInfo);

		submission_batch& operator+=(submission_batch aSubmissionBatch);
		submission_batch& operator+=(const std::vector<submission_batch>& aSubmissionBatch);

	};

	// Builds submission infos from command_batchs scripted by render targets.
	submission_batch build(const std::vector<command_batch>& aCommandBatch);

}

#endif // !GEODESY_CORE_GCL_SUBMISSION_BATCH_H
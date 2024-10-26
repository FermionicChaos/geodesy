#pragma once
#ifndef GEODESY_CORE_LGC_THREAD_TOOLS_H
#define GEODESY_CORE_LGC_THREAD_TOOLS_H

#include <cstddef>

#include <vector>

namespace geodesy::core::lgc {

	struct workload {
		size_t Offset;
		size_t Size;
	};

	// Calculates Per Thread Workload, evenly distributing work
	std::vector<workload> calculate_workloads(size_t aElementCount, size_t aThreadCount);

}

#endif // !GEODESY_CORE_LGC_THREAD_TOOLS_H

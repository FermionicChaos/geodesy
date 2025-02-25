#pragma once
#ifndef GEODESY_CORE_GCL_ACCELERATION_STRUCTURE_H
#define GEODESY_CORE_GCL_ACCELERATION_STRUCTURE_H

#include "../../config.h"
#include "config.h"
#include "buffer.h"

namespace geodesy::core::gcl {
	
	class acceleration_structure {
	public:

		std::shared_ptr<context> 			Context;
		std::shared_ptr<buffer>				AccelerationStructureBuffer;
		std::shared_ptr<buffer> 			ScratchBuffer;
		VkAccelerationStructureKHR			AccelerationStructure;
		VkDeviceAddress						AccelerationStructureDeviceAddress;

		acceleration_structure();
		// Build Bottom Level AS (Mesh Geometry Data).
		acceleration_structure(std::shared_ptr<context> aContext, std::shared_ptr<buffer> aVertexBuffer);
		// Build Top Level AS (Mesh Instances).

		~acceleration_structure();

	};

}

#endif // GEODESY_CORE_GCL_ACCELERATION_STRUCTURE_H
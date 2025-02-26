#pragma once
#ifndef GEODESY_CORE_GCL_ACCELERATION_STRUCTURE_H
#define GEODESY_CORE_GCL_ACCELERATION_STRUCTURE_H
/*
An acceleration structure in vulkan is an object that maps out a geometry and optimizes it for ray tracing.
This is a bottom level acceleration structure that maps out the geometry of a mesh.
*/

#include "../../config.h"
#include "config.h"
#include "buffer.h"

namespace geodesy::core::gcl {
	
	class acceleration_structure {
	public:

		std::shared_ptr<context> 			Context;
		std::shared_ptr<buffer>				Buffer;
		std::shared_ptr<buffer> 			UpdateScratchBuffer;
		std::shared_ptr<buffer> 			BuildScratchBuffer;
		VkAccelerationStructureKHR			AccelerationStructure;
		VkDeviceAddress						AccelerationStructureDeviceAddress;

		acceleration_structure();
		// Build Bottom Level AS (Mesh Geometry Data).
		acceleration_structure(std::shared_ptr<context> aContext, const gfx::mesh* aDeviceMesh, const gfx::mesh* aHostMesh);
		// Build Top Level AS (Mesh Instances).

		// Clear out resources
		~acceleration_structure();

	};

}

#endif // GEODESY_CORE_GCL_ACCELERATION_STRUCTURE_H
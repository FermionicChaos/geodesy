#include <geodesy/core/gpu/acceleration_structure.h>

#include <geodesy/core/gpu/context.h>
#include <geodesy/core/gfx/mesh.h>
#include <geodesy/core/gfx/node.h>
#include <geodesy/core/gfx/model.h>
#include <geodesy/runtime/object.h>
#include <geodesy/runtime/stage.h>

namespace geodesy::core::gpu {

	acceleration_structure::acceleration_structure() {
		// Zero init here.
		this->Context = nullptr;
		
	}

	acceleration_structure::acceleration_structure(std::shared_ptr<context> aContext, const gfx::mesh* aDeviceMesh, const gfx::mesh* aHostMesh) : acceleration_structure() {
		this->Context = aContext;
		uint32_t PrimitiveCount = (aHostMesh->Vertex.size() <= (1 << 16) ? aHostMesh->Topology.Data16.size() : aHostMesh->Topology.Data32.size()) / 3;
		// Build Bottom Level AS (Mesh Geometry Data).
		// Needed for device addresses.
		// PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)Context->FunctionPointer["vkGetBufferDeviceAddress"];
		///*
		// Describe the geometry of the acceleration structure.
		VkAccelerationStructureGeometryKHR ASG{};
		ASG.sType												= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		ASG.pNext												= NULL;
		ASG.geometryType										= VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		ASG.geometry.triangles.sType							= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		ASG.geometry.triangles.pNext							= NULL;
		ASG.geometry.triangles.vertexFormat						= VK_FORMAT_R32G32B32_SFLOAT;
		ASG.geometry.triangles.vertexData.deviceAddress 		= aDeviceMesh->VertexBuffer->device_address();
		ASG.geometry.triangles.vertexStride						= sizeof(gfx::mesh::vertex);
		ASG.geometry.triangles.maxVertex						= aHostMesh->Vertex.size();
		ASG.geometry.triangles.indexType						= aHostMesh->Vertex.size() <= (1 << 16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		ASG.geometry.triangles.indexData.deviceAddress 			= aDeviceMesh->IndexBuffer->device_address();
		ASG.geometry.triangles.transformData.deviceAddress		= 0;
		ASG.flags												= VK_GEOMETRY_OPAQUE_BIT_KHR;

		// Needed for acceleration structure creation.
		VkAccelerationStructureBuildGeometryInfoKHR ASBGI{};
		ASBGI.sType												= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		ASBGI.pNext												= NULL;
		ASBGI.type												= VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		ASBGI.flags												= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		ASBGI.mode												= VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		ASBGI.srcAccelerationStructure							= VK_NULL_HANDLE;
		ASBGI.dstAccelerationStructure							= VK_NULL_HANDLE;
		ASBGI.geometryCount										= 1;
		ASBGI.pGeometries										= &ASG;
		ASBGI.ppGeometries										= NULL;
		ASBGI.scratchData.deviceAddress							= 0;

		// Get the required sizes for the acceleration structure.
		VkAccelerationStructureBuildSizesInfoKHR ASBSI{};
		ASBSI.sType												= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		ASBSI.pNext												= NULL;
		if (aContext->FunctionPointer.count("vkGetAccelerationStructureBuildSizesKHR") > 0) {
			PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)aContext->FunctionPointer["vkGetAccelerationStructureBuildSizesKHR"];
			vkGetAccelerationStructureBuildSizesKHR(aContext->Handle, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI, &PrimitiveCount, &ASBSI);

			// Build the acceleration structure buffer.
			gpu::buffer::create_info ASBCI;
			ASBCI.Memory = device::memory::DEVICE_LOCAL;
			ASBCI.Usage = buffer::usage::ACCELERATION_STRUCTURE_STORAGE_KHR | buffer::usage::SHADER_DEVICE_ADDRESS | buffer::usage::STORAGE | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
			this->Buffer = aContext->create_buffer(ASBCI, ASBSI.accelerationStructureSize);

			// TODO: Add support for updating acceleration structures?

			// Scratch buffer is used for temporary storage during acceleration structure build.
			gpu::buffer::create_info SBCI;
			SBCI.Memory = device::memory::DEVICE_LOCAL;
			SBCI.Usage = buffer::usage::SHADER_DEVICE_ADDRESS | buffer::usage::STORAGE | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
			this->BuildScratchBuffer = aContext->create_buffer(SBCI, ASBSI.buildScratchSize);
		}

		VkAccelerationStructureCreateInfoKHR ASCI{};
		ASCI.sType 				= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		ASCI.pNext 				= NULL;
		ASCI.createFlags 		= 0;
		ASCI.buffer 			= this->Buffer->Handle;
		ASCI.offset 			= 0;
		ASCI.size 				= ASBSI.accelerationStructureSize;  // Size we got from vkGetAccelerationStructureBuildSizesKHR
		ASCI.type 				= VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		ASCI.deviceAddress 		= 0;  // This is for capturing device address during creation, typically not needed

		PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)aContext->FunctionPointer["vkCreateAccelerationStructureKHR"];
		VkResult Result = vkCreateAccelerationStructureKHR(aContext->Handle, &ASCI, NULL, &this->Handle);

		if (Result == VK_SUCCESS) {
			// Update ASBGI with the acceleration structure handle.
			ASBGI.dstAccelerationStructure = this->Handle;
			// Update ASBGI with the scratch buffer device address.
			ASBGI.scratchData.deviceAddress = this->Buffer->device_address();

			// Now fill out acceleration structure buffer.
			VkAccelerationStructureBuildRangeInfoKHR ASBRI;
			ASBRI.primitiveCount 	= PrimitiveCount;
			ASBRI.primitiveOffset 	= 0;
			ASBRI.firstVertex 		= 0;
			ASBRI.transformOffset 	= 0;

			VkMemoryBarrier Barrier{};
			Barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			Barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
			Barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

			VkCommandBuffer CommandBuffer = aContext->allocate_command_buffer(device::operation::GRAPHICS_AND_COMPUTE);
			PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)aContext->FunctionPointer["vkCmdBuildAccelerationStructuresKHR"];
			const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &ASBRI;
			Result = aContext->begin(CommandBuffer);
			vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &ASBGI, &pBuildRangeInfo);
			vkCmdPipelineBarrier(
				CommandBuffer,
				VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				0,
				1, &Barrier,
				0, NULL,
				0, NULL
			);
			Result = aContext->end(CommandBuffer);
			aContext->execute_and_wait(device::operation::GRAPHICS_AND_COMPUTE, CommandBuffer);
			aContext->release_command_buffer(device::operation::GRAPHICS_AND_COMPUTE, CommandBuffer);

			// Get the device address of the acceleration structure.
			this->DeviceAddress = this->device_address();
		}
	}

	acceleration_structure::acceleration_structure(std::shared_ptr<context> aContext, const runtime::stage* aStage) {
		// TLAS will have to iterate through every objects model to gather its mesh instances.

		// Vulkan mesh instance list over entire stage.
		std::vector<VkAccelerationStructureInstanceKHR> InstanceList;

		// Iterate through all objects in the stage.
		for (const auto& Object : aStage->Object) {
			// Get list of mesh instances per object model.
			std::vector<gfx::mesh::instance*> MeshInstanceList = Object->gather_instances();
			for (gfx::mesh::instance* MeshInstance : MeshInstanceList) {
				VkAccelerationStructureInstanceKHR ASI{};
				gfx::mesh* Mesh = Object->Model->Mesh[MeshInstance->MeshIndex].get();

				// ! Object Transformation Info.
				// Create Translation Matrix.
				math::mat<float, 4, 4> ObjectTranslation = {
					1.0f, 0.0f, 0.0f, Object->Position[0],
					0.0f, 1.0f, 0.0f, Object->Position[1],
					0.0f, 0.0f, 1.0f, Object->Position[2],
					0.0f, 0.0f, 0.0f, 1.0f
				};
				// Create Orientation Matrix.
				math::mat<float, 4, 4> ObjectOrientation = {
					Object->DirectionRight[0], 		Object->DirectionFront[0], 		Object->DirectionUp[0], 	0.0f,
					Object->DirectionRight[1], 		Object->DirectionFront[1], 		Object->DirectionUp[1], 	0.0f,
					Object->DirectionRight[2], 		Object->DirectionFront[2], 		Object->DirectionUp[2], 	0.0f,
					0.0f, 							0.0f, 							0.0f, 						1.0f
				};
				// Create Scale Matrix.
				math::mat<float, 4, 4> ObjectScale = {
					Object->Scale[0], 	0.0f, 				0.0f, 				0.0f,
					0.0f, 				Object->Scale[1], 	0.0f, 				0.0f,
					0.0f, 				0.0f, 				Object->Scale[2], 	0.0f,
					0.0f, 				0.0f, 				0.0f, 				1.0f
				};
				math::mat<float, 4, 4> ObjectTransform = ObjectTranslation * ObjectOrientation * ObjectScale;

				// Matrix Transform, get global transform to world space. (Include Object Transform.)
				math::mat<float, 4, 4> WorldTransform = ObjectTransform * MeshInstance->Parent->transform();
				for (int Row = 0; Row < 3; Row++) {
					for (int Col = 0; Col < 4; Col++) {
						// Convert from memory internal format to vulkan using proper accessors.
						ASI.transform.matrix[Row][Col] = WorldTransform(Row, Col);
					}
				}

				// This can be used to specify geometry specific data. (Used often for material IDs)
				ASI.instanceCustomIndex							= 0;
				// Visible to all rays.
				ASI.mask										= 0xFF;
				// Might need info from ray tracing pipeline.
				ASI.instanceShaderBindingTableRecordOffset		; // TODO: Figure out relation with pipeline.
				// Flags
				ASI.flags										= VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
				// Instance ID.
				ASI.accelerationStructureReference				= Mesh->AccelerationStructure->DeviceAddress;

				// Add to list in TLAS.
				InstanceList.push_back(ASI);
			}
		}

		// Create Instance Buffer for TLAS.
		{
			gpu::buffer::create_info IBCI;
			IBCI.Memory = device::memory::DEVICE_LOCAL;
			IBCI.Usage = buffer::usage::ACCELERATION_STRUCTURE_STORAGE_KHR | buffer::usage::SHADER_DEVICE_ADDRESS | buffer::usage::STORAGE | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
			this->InstanceBuffer = aContext->create_buffer(IBCI, InstanceList.size() * sizeof(VkAccelerationStructureInstanceKHR), InstanceList.data());
		}

		// Define geometry for TLAS, similar struct to BLAS.
		VkAccelerationStructureGeometryKHR ASG{};
		ASG.sType												= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		ASG.pNext												= NULL;
		ASG.geometryType										= VK_GEOMETRY_TYPE_INSTANCES_KHR;
		ASG.geometry.instances.sType							= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		ASG.geometry.instances.pNext							= NULL;
		ASG.geometry.instances.arrayOfPointers					= VK_FALSE;
		ASG.geometry.instances.data.deviceAddress				= this->InstanceBuffer->device_address();
		ASG.flags												= VK_GEOMETRY_OPAQUE_BIT_KHR;

		// Build Geometry Info for TLAS.
		VkAccelerationStructureBuildGeometryInfoKHR ASBGI{};
		ASBGI.sType												= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		ASBGI.pNext												= NULL;
		ASBGI.type												= VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		ASBGI.flags												= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR; // TODO: change to fast build since we update often for mesh anims.
		ASBGI.mode												= VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		ASBGI.srcAccelerationStructure							= VK_NULL_HANDLE;
		ASBGI.dstAccelerationStructure							= VK_NULL_HANDLE;
		ASBGI.geometryCount										= 1;
		ASBGI.pGeometries										= &ASG;
		ASBGI.ppGeometries										= NULL;
		ASBGI.scratchData.deviceAddress							= 0;	

		// Get build sizes based on geometry info provided.
		VkAccelerationStructureBuildSizesInfoKHR ASBSI{};
		ASBSI.sType												= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		ASBSI.pNext												= NULL;
		if (aContext->FunctionPointer.count("vkGetAccelerationStructureBuildSizesKHR") > 0) {
			uint32_t PrimitiveCount = InstanceList.size();
			PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)aContext->FunctionPointer["vkGetAccelerationStructureBuildSizesKHR"];
			vkGetAccelerationStructureBuildSizesKHR(aContext->Handle, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &ASBGI, &PrimitiveCount, &ASBSI);

			// Build the acceleration structure buffer.
			gpu::buffer::create_info ASBCI;
			ASBCI.Memory = device::memory::DEVICE_LOCAL;
			ASBCI.Usage = buffer::usage::ACCELERATION_STRUCTURE_STORAGE_KHR | buffer::usage::SHADER_DEVICE_ADDRESS | buffer::usage::STORAGE | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
			this->Buffer = aContext->create_buffer(ASBCI, ASBSI.accelerationStructureSize);

			// TODO: Add support for updating acceleration structures?

			// Scratch buffer is used for temporary storage during acceleration structure build.
			gpu::buffer::create_info SBCI;
			SBCI.Memory = device::memory::DEVICE_LOCAL;
			SBCI.Usage = buffer::usage::SHADER_DEVICE_ADDRESS | buffer::usage::STORAGE | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
			this->BuildScratchBuffer = aContext->create_buffer(SBCI, ASBSI.buildScratchSize);
		}

		VkAccelerationStructureCreateInfoKHR ASCI{};
		ASCI.sType					= VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		ASCI.pNext					= NULL;
		ASCI.createFlags			= 0;
		ASCI.buffer					= this->Buffer->Handle;
		ASCI.offset					= 0;
		ASCI.size					= ASBSI.accelerationStructureSize;
		ASCI.type					= VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		ASCI.deviceAddress			= 0;

		// Create Top Level Acceleration Structure.
		PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)aContext->FunctionPointer["vkCreateAccelerationStructureKHR"];
		VkResult Result = vkCreateAccelerationStructureKHR(aContext->Handle, &ASCI, NULL, &this->Handle);

		// Now fill out acceleration structure buffer.
		if (Result == VK_SUCCESS) {
			// Update ASBGI with the acceleration structure handle.
			ASBGI.dstAccelerationStructure = this->Handle;
			// Update ASBGI with the scratch buffer device address.
			ASBGI.scratchData.deviceAddress = this->BuildScratchBuffer->device_address();

			// Now fill out acceleration structure buffer.
			VkAccelerationStructureBuildRangeInfoKHR ASBRI;
			ASBRI.primitiveCount 	= InstanceList.size();
			ASBRI.primitiveOffset 	= 0;
			ASBRI.firstVertex 		= 0;
			ASBRI.transformOffset 	= 0;

			VkMemoryBarrier Barrier{};
			Barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			Barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
			Barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

			VkCommandBuffer CommandBuffer = aContext->allocate_command_buffer(device::operation::GRAPHICS_AND_COMPUTE);
			PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)aContext->FunctionPointer["vkCmdBuildAccelerationStructuresKHR"];
			const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &ASBRI;
			Result = aContext->begin(CommandBuffer);
			vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &ASBGI, &pBuildRangeInfo);
			vkCmdPipelineBarrier(
				CommandBuffer,
				VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
				0,
				1, &Barrier,
				0, NULL,
				0, NULL
			);
			Result = aContext->end(CommandBuffer);
			aContext->execute_and_wait(device::operation::GRAPHICS_AND_COMPUTE, CommandBuffer);
			aContext->release_command_buffer(device::operation::GRAPHICS_AND_COMPUTE, CommandBuffer);

			// Get the device address of the acceleration structure.
			this->DeviceAddress = this->device_address();
		}
	}

	acceleration_structure::~acceleration_structure() {

	}

	VkDeviceAddress acceleration_structure::device_address() const {
		// Get AS device address.
		VkAccelerationStructureDeviceAddressInfoKHR ASDAI{};
		ASDAI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		ASDAI.accelerationStructure = this->Handle;
		PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)this->Context->FunctionPointer["vkGetAccelerationStructureDeviceAddressKHR"];
		return vkGetAccelerationStructureDeviceAddressKHR(this->Context->Handle, &ASDAI);
	}

}
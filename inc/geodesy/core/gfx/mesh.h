#pragma once
#ifndef GEODESY_CORE_GFX_MESH_H
#define GEODESY_CORE_GFX_MESH_H

#include <memory>

// Geodesy Engine Core
#include "../../config.h"

// Base Mesh Object
#include "../phys.h"

// GPU Resource Management
#include "../gpu/context.h"
#include "../gpu/buffer.h"
#include "../gpu/pipeline.h"

#define MAX_BONE_COUNT 256

struct aiMesh;

namespace geodesy::core::gfx {

	class mesh : public phys::mesh {
	public:

		struct instance {

			struct uniform_data {
				alignas(16) math::mat<float, 4, 4> Transform;
				alignas(16) math::mat<float, 4, 4> BoneTransform[MAX_BONE_COUNT];
				alignas(16) math::mat<float, 4, 4> BoneOffset[MAX_BONE_COUNT];
				uniform_data();
				uniform_data(const mesh::instance* aInstance);
			};

			// Access to node hierarchy.
			phys::node* 					Root;
			phys::node* 					Parent; // The node the mesh instance exists in.
			// Has no children, so this is always empty.

			// Host Memory Reference
			std::vector<vertex::weight> 	Vertex; // Contains Per Vertex BoneIDs & BoneWeights. (Goes to the vertex buffer)
			std::vector<bone>				Bone; // Contains Per Bone/Node data specifying which vertices it influences. (Goes to bone uniform buffer)
			
			// Device Memory Objects
			std::shared_ptr<gpu::context> 	Context;
			std::shared_ptr<gpu::buffer> 	VertexWeightBuffer;
			std::shared_ptr<gpu::buffer> 	UniformBuffer;

			// Add reference to parent node in hierarchy.
			int 							MeshIndex;
			uint 							MaterialIndex;
			
			instance();
			instance(uint aVertexCount, const std::vector<bone>& aBoneData, int aMeshIndex, uint aMaterialIndex, phys::node* aRoot = nullptr, phys::node* aParent = nullptr);
			instance(std::shared_ptr<gpu::context> aContext, const instance& aInstance, phys::node* aRoot = nullptr, phys::node* aParent = nullptr);
			
		};

		// Host Memory Reference
		std::weak_ptr<mesh> 							HostMesh;

		// Device Memory Objects
		std::shared_ptr<gpu::context> 					Context;
		std::shared_ptr<gpu::buffer> 					VertexBuffer;
		std::shared_ptr<gpu::buffer>					IndexBuffer;
		std::shared_ptr<gpu::acceleration_structure> 	AccelerationStructure;

		mesh();
		mesh(const aiMesh* aMesh);
		mesh(std::shared_ptr<gpu::context> aContext, std::shared_ptr<mesh> aMesh);

	};

}

#endif // !GEODESY_CORE_GFX_MESH_H

#pragma once
#ifndef GEODESY_CORE_GFX_MESH_H
#define GEODESY_CORE_GFX_MESH_H

#include <memory>

#include "../phys/mesh.h"

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

			// Host Memory Objects
			int 							Index;
			math::mat<float, 4, 4> 			Transform;
			std::vector<vertex::weight> 	Vertex;
			std::vector<bone>				Bone;				// Yes, this is tied to the instance rather than the mesh object.
			uint 							MaterialIndex;		// Yes, this is tied to the instance rather than the mesh object.

			// Device Memory Objects
			std::shared_ptr<gpu::context> 	Context;
			std::shared_ptr<gpu::buffer> 	VertexWeightBuffer;
			std::shared_ptr<gpu::buffer> 	UniformBuffer;

			instance();
			instance(int aMeshIndex, math::mat<float, 4, 4> aTransform, uint aVertexCount, const std::vector<bone>& aBoneData, uint aMaterialIndex);
			instance(std::shared_ptr<gpu::context> aContext, const instance& aInstance);

			void update(double DeltaTime);
			
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
		// mesh(std::shared_ptr<gpu::context> aContext, const std::vector<vertex>& aVertexData, const topology& aTopologyData);
		mesh(std::shared_ptr<gpu::context> aContext, std::shared_ptr<mesh> aMesh);

		// void draw(VkCommandBuffer aCommandBuffer, std::shared_ptr<gpu::pipeline> aPipeline, std::shared_ptr<gpu::framebuffer> aFramebuffer, std::shared_ptr<gpu::descriptor::array> aDescriptorArray);

		// void draw(std::vector<std::shared_ptr<gpu::image>> aOutput, std::shared_ptr<gpu::pipeline> aPipeline, std::shared_ptr<material> aMaterial);

	};

}

#endif // !GEODESY_CORE_GFX_MESH_H

#pragma once
#ifndef GEODESY_CORE_GFX_MESH_H
#define GEODESY_CORE_GFX_MESH_H

#include <memory>

#include "../phys/mesh.h"

#include "../gcl/context.h"
#include "../gcl/buffer.h"
#include "../gcl/pipeline.h"

struct aiMesh;

namespace geodesy::core::gfx {

	class mesh : public phys::mesh {
	public:

		struct instance {

			// Host Memory Objects
			int 							Index;
			math::mat<float, 4, 4> 			Transform;
			std::vector<vertex::weight> 	Vertex;
			std::vector<bone>				Bone;				// Yes, this is tied to the instance rather than the mesh object.
			uint 							MaterialIndex;		// Yes, this is tied to the instance rather than the mesh object.

			// Device Memory Objects
			std::shared_ptr<gcl::context> 	Context;
			std::shared_ptr<gcl::buffer> 	VertexWeightBuffer;
			std::shared_ptr<gcl::buffer> 	UniformBuffer;

			instance();
			instance(int aMeshIndex, math::mat<float, 4, 4> aTransform, uint aVertexCount, const std::vector<bone>& aBoneData, uint aMaterialIndex);
			instance(std::shared_ptr<gcl::context> aContext, const instance& aInstance);

			void update(double DeltaTime);
			
		};

		// Device Memory Objects
		std::shared_ptr<gcl::context> 		Context;
		std::shared_ptr<gcl::buffer> 		VertexBuffer;
		std::shared_ptr<gcl::buffer>		IndexBuffer;
		std::shared_ptr<gcl::buffer>		UniformBuffer;

		mesh(const aiMesh* aMesh);
		mesh(std::shared_ptr<gcl::context> aContext, const std::vector<vertex>& aVertexData, const topology& aTopologyData);
		mesh(std::shared_ptr<gcl::context> aContext, std::shared_ptr<mesh> aMesh);

		// void draw(VkCommandBuffer aCommandBuffer, std::shared_ptr<gcl::pipeline> aPipeline, std::shared_ptr<gcl::framebuffer> aFramebuffer, std::shared_ptr<gcl::descriptor::array> aDescriptorArray);

		// void draw(std::vector<std::shared_ptr<gcl::image>> aOutput, std::shared_ptr<gcl::pipeline> aPipeline, std::shared_ptr<material> aMaterial);

	};

}

#endif // !GEODESY_CORE_GFX_MESH_H

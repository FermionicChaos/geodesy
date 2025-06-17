#pragma once
#ifndef GEODESY_CORE_GFX_NODE_H
#define GEODESY_CORE_GFX_NODE_H

// Engine Configuration
#include "../../config.h"

// Physics Base
#include "../phys.h"

#include "mesh.h"
#include "material.h"

namespace geodesy::core::gfx {

	class node : public phys::node {
	public:

		std::shared_ptr<gpu::context> Context;
		std::vector<mesh::instance> MeshInstance; // Mesh Instance located in node hierarchy.

		node();
		node(const aiScene* aScene, const aiNode* aNode, phys::node* aRoot = nullptr, phys::node* aParent = nullptr);
		node(std::shared_ptr<gpu::context> aContext, const node* aNode, phys::node* aRoot = nullptr, phys::node* aParent = nullptr);
		~node();

		void copy(const phys::node* aNode) override;
		void update(
			double 									aDeltaTime = 0.0f, 
			double 									aTime = 0.0f, 
			const std::vector<float>& 				aAnimationWeight = { 1.0f }, 
			const std::vector<phys::animation>& 	aPlaybackAnimation = {}
		);

		// Counts the total number of mesh references in the tree.
		size_t instance_count();

		// Gather all mesh instances in the hierarchy.
		std::vector<gfx::mesh::instance*> gather_instances();


	};

}

#endif // !GEODESY_CORE_GFX_NODE_H
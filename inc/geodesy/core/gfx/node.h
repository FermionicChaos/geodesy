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

		std::vector<mesh::instance> MeshInstance; // Mesh Instance located in node hierarchy.

		node();
		node(const aiScene* aScene, const aiNode* aNode);
		node(std::shared_ptr<gpu::context> aContext, const node* aNode);
		node(const node& aInput);
		node(node&& aInput) noexcept;
		~node();

		node& operator=(const node& aRhs);
		node& operator=(node&& aRhs) noexcept;

		// Counts the total number of mesh references in the tree.
		size_t instance_count();

		// Gather all mesh instances in the hierarchy.
		std::vector<gfx::mesh::instance*> gather_instances();

		// This is overwritten so mesh instance gpu buffers can be updated.
		//void update(const std::vector<float>& aAnimationWeight = { 1.0f }, const std::vector<phys::animation>& aPlaybackAnimation = {}, double aTime = 0.0f);

	};

}

#endif // !GEODESY_CORE_GFX_NODE_H
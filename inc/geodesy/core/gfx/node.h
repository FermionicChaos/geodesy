#pragma once
#ifndef GEODESY_CORE_GFX_NODE_H
#define GEODESY_CORE_GFX_NODE_H

#include "../../config.h"

#include "mesh.h"
#include "material.h"
#include "animation.h"

struct aiScene;
struct aiNode;

namespace geodesy::core::gfx {

	class node {
	public:

		// Traversal Data
		node*								Root;
		node*								Parent;
		std::vector<node>					Child;

		// Metadata
		std::string							Name;				// Name of the Node in Hierarchy
		math::mat<float, 4, 4>				Transformation;		// This transforms to parent node space.
		std::vector<mesh::instance> 		MeshInstance; 		// Mesh Instance located in node hierarchy.

		node();
		node(const aiScene* aScene, const aiNode* aNode);
		node(std::shared_ptr<gcl::context> aContext, const node& aNode);
		node(const node& aInput);
		node(node&& aInput) noexcept;
		~node();

		node& operator=(const node& aRhs);
		node& operator=(node&& aRhs) noexcept;

		node& operator[](int aIndex);

		// Finds node with name in hierarchy.
		node* find(std::string aName);
		// Update the node hierarchy. (Applies Node & Mesh Animations)
		void update(const std::vector<float>& aAnimationWeight = { 1.0f }, const std::vector<animation>& aPlaybackAnimation = {}, double aTime = 0.0f);	
		// Total Number of Nodes from this point on.
		size_t node_count() const;
		// Counts the total number of mesh references in the tree.
		size_t instance_count() const;
		// For this node, it will calculate the model transform for a node at a particular time.
		math::mat<float, 4, 4> transform(const std::vector<float>& aAnimationWeight = { 1.0f }, const std::vector<animation>& aPlaybackAnimation = {}, double aTime = 0.0f);
		// Gathers a list of references to MeshInstance objects.
		std::vector<mesh::instance*> gather_mesh_instances();

	};

}

#endif // !GEODESY_CORE_GFX_NODE_H
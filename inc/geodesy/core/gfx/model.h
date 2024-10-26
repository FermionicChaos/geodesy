#pragma once
#ifndef GEODESY_CORE_GFX_MODEL_H
#define GEODESY_CORE_GFX_MODEL_H

#include <memory>

#include "../../config.h"

#include "../io/file.h"

#include "mesh.h"
#include "material.h"
#include "animation.h"

namespace geodesy::core::gfx {

	class model : public io::file {
	public:

		struct node {

			// Traversal Data
			node*								Root;
			node*								Parent;
			std::vector<node>					Child;

			// Metadata
			std::string							Name;				// Name of the Node in Hierarchy
			float 								Weight;				// Default Weight of the Node is 1.0f. 
			math::mat<float, 4, 4>				Transformation;		// Static Bone to Model Space Transform Component = T0*T1*T2*...*Tn*Vbs
			std::vector<animation> 				Animation; 			// Overrides 
			std::vector<mesh::instance> 		MeshInstance; 		//

			node();
			node(const node& aInput);
			node(node&& aInput) noexcept;
			~node();

			node& operator=(const node& aRhs);
			node& operator=(node&& aRhs) noexcept;

			node& operator[](int aIndex);
			node& operator[](const char* aName);

			// Update the node hierarchy. (Applies Node & Mesh Animations)
			void update(double aTime);	
			// Total Number of Nodes from this point on.
			size_t node_count() const;
			// Counts the total number of mesh references in the tree.
			size_t instance_count() const;
			// For this node, it will calculate the model transform for a node at a particular time.
			// It uses the node's animation data to calculate the transform.
			math::mat<float, 4, 4> global_transform(double aTime);
			// Gathers a list of references to MeshInstance objects.
			std::vector<mesh::instance*> gather_mesh_instances();

		private:

			void set_root(node* aRoot);

			void clear();

			void zero_out();

		};

		static bool initialize();
		static void terminate();

		// --------------- Aggregate Model Resources --------------- //

		// Node Hierarchy
		std::string										Name;
		node											Hierarchy;			// Root Node Hierarchy 
		double 											Time;

		// Resources
		std::shared_ptr<gcl::context> 					Context;
		std::vector<std::shared_ptr<mesh>> 				Mesh;
		std::vector<std::shared_ptr<material>> 			Material;
		std::vector<std::shared_ptr<gcl::image>> 		Texture;
		// std::vector<std::shared_ptr<light>> 			Light;				// Not Relevant To Model, open as stage.
		// std::vector<std::shared_ptr<camera>> 		Camera;			// Not Relevant To Model, open as stage.
		std::shared_ptr<gcl::buffer> 					UniformBuffer;

		model();
		model(std::string aFilePath, file::manager* aFileManager = nullptr);
		model(std::shared_ptr<gcl::context> aContext, std::string aFilePath);
		model(std::shared_ptr<gcl::context> aContext, gcl::image::create_info aCreateInfo, std::shared_ptr<model> aModel);
		~model();

		void update(double aDeltaTime);

	};

}

#endif // !GEODESY_CORE_GFX_MODEL_H

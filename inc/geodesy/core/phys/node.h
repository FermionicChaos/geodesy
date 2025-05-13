#pragma once
#ifndef GEODESY_CORE_PHYS_NODE_H
#define GEODESY_CORE_PHYS_NODE_H

// Include include config.
#include "../../config.h"
// Include include math.
#include "../math.h"
// Include physics mesh. (Needed for collisions)
#include "mesh.h"

namespace geodesy::core::phys {

	/*
	The goal of phys::node is that it is the base node object for a node hierarchy with transformations.
	It's intended use is to first be used as a container class for unpacked 3d models from assimp. Second,
	a base primitive object in which rigid body physics can be applied to. A node can be a base physics node,
	it can be a graphics node carrying mesh instances, it can be a bone which animates vertex data in a vertex
	shader, and finally a root node can be an object which the engine processes and applies physics to.
	*/

	///*
	class node {
	public:

		// Node Types to identify runtime node.
		enum type {
			PHYSICS,
			GRAPHICS,
			BONE,
			OBJECT,
		};

		enum motion {
			STATIC,			// Node doesn't move in world space
			DYNAMIC,		// Node moves, but based on physical forces applied.
			ANIMATED,		// Node moves based on predetermined animation path data.
		};
		
		// Node metadata/hierarchy.
		std::string             				Name;       		// Node name
		node*                   				Root;       		// Root node in hierarchy
		node*                   				Parent;     		// Parent node in hierarchy
		std::vector<std::shared_ptr<node>>      Child;      		// Child nodes in hierarchy
		
		// Node Data
		float									Time;				// Second 			[s]
		float 									DeltaTime; 			// Second 			[s]
		float									Mass;				// Kilogram			[kg]
		math::mat<float, 3, 3>					InertiaTensor;		// Inertia Tensor	[kg*m^2]
		math::vec<float, 3>						Position;			// Meter			[m]
		math::quaternion<float>					Orientation;		// Quaternion		[Dimensionless]
		math::vec<float, 3> 					Scale;				// Scaling Factor	[Dimensionless]
		math::vec<float, 3>						LinearMomentum;		// Linear Momentum	[kg*m/s]
		math::vec<float, 3>						AngularMomentum;	// Angular Momentum [kg*m/s]
		math::mat<float, 4, 4> 					Transformation; 	// Node transformation matrix
		std::shared_ptr<phys::mesh>				CollisionMesh;		// Mesh Data
		
		node();
		node(const node& aInput);
		node(node&& aInput) noexcept;
		~node();
		
		node& operator=(const node& aRhs);
		node& operator=(node&& aRhs) noexcept;

		virtual void copy(const node* aInput);
		virtual void swap(node* aInput);
		virtual void update(float aDeltaTime);
		
	};
	//*/
	
}

#endif // !GEODESY_CORE_PHYS_NODE_H
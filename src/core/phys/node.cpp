#include <geodesy/core/phys/node.h>

#include <geodesy/core/gfx/node.h>
#include <geodesy/runtime/object.h>

// Model Loading
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace geodesy::core::phys {

	// Default constructor, zero out all data.
	node::node() {
		this->Identifier 				= "";
		this->Type 				= node::PHYSICS; // Default type to PHYSICS.
		this->Root 				= this;
		this->Parent 			= nullptr;
		this->Mass 				= 1.0f; // Default mass to 1 kg.
		this->InertiaTensor 	= {
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f
		};
		this->Position 			= { 0.0f, 0.0f, 0.0f }; // Default position to origin.
		this->Orientation 		= { 1.0f, 0.0f, 0.0f, 0.0f }; // Default orientation to identity quaternion.
		this->Scale 			= { 1.0f, 1.0f, 1.0f }; // Default scale to 1 in all dimensions.
		this->LinearMomentum 	= { 0.0f, 0.0f, 0.0f }; // Default linear momentum to zero.
		this->AngularMomentum 	= { 0.0f, 0.0f, 0.0f }; // Default angular momentum to zero.
		this->DefaultTransform 	= {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		this->CurrentTransform = this->DefaultTransform;
		this->GlobalTransform = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}

	node::~node() {
		// Clear all child nodes memory.
		for (auto& C : this->Child) {
			delete C; // Delete each child node.
		}
	}

	size_t node::node_count() const {
		size_t TotalCount = 1;
		for (auto Chd : Child) {
			TotalCount += Chd->node_count();
		}
		return TotalCount;
	}

	// The main transform function that calculates the model transform for this node.
	math::mat<float, 4, 4> node::transform() const {
		// Recursively calculates the world transform for this node using current state of the node hierarchy.
		if (this->Root != this) {
			return this->Parent->transform() * this->CurrentTransform;
		}
		else {
			return this->CurrentTransform; // If this is the root node, return the current transform.
		}
	}

	node* node::find(std::string aName) {
		// Find the node with the given name in the hierarchy.
		if (this->Identifier == aName) {
			return this;
		}
		for (const auto& child : this->Child) {
			node* FoundNode = child->find(aName);
			if (FoundNode != nullptr) {
				return FoundNode;
			}
		}
		return nullptr; // Return nullptr if not found.
	}

	std::vector<node*> node::linearize() {
		std::vector<node*> Nodes;
		Nodes.push_back(this);
		for (auto Chd : this->Child) {
			std::vector<node*> CNodes = Chd->linearize();
			Nodes.insert(Nodes.end(), CNodes.begin(), CNodes.end());
		}
		return Nodes;
	}

	void node::set_root(node* aRootNode) {
		this->Root = aRootNode; // Set the root node for this node.
		for (auto& Chd : this->Child) {
			Chd->set_root(aRootNode);
		}
	}

	void node::copy_data(const node* aNode) {
		// This function simply copies all data not related to the hierarchy.
		// This is used to copy data from one node to another.
		this->Identifier = aNode->Identifier;
		this->Type = aNode->Type;
		this->Mass = aNode->Mass;
		this->InertiaTensor = aNode->InertiaTensor;
		this->Position = aNode->Position;
		this->Orientation = aNode->Orientation;
		this->Scale = aNode->Scale;
		this->LinearMomentum = aNode->LinearMomentum;
		this->AngularMomentum = aNode->AngularMomentum;
		this->DefaultTransform = aNode->DefaultTransform;
		this->CurrentTransform = aNode->CurrentTransform; // Copy the current transform.
		this->GlobalTransform = aNode->GlobalTransform; // Copy the global transform.
		this->CollisionMesh = aNode->CollisionMesh; // Copy the collision mesh if it exists.
	}

	void node::copy(const node* aNode) {}

	void node::swap(node* aNode) {
		// Swap the data of this node with the given node.
		if (aNode == nullptr) {
			return; // Nothing to swap with.
		}
		// Copy node data
		this->copy_data(aNode);
		this->Child.swap(aNode->Child);
		for (auto& Chd : this->Child) {
			// Set parent for immediate children
			Chd->Parent = this;
			// Set root for each child node tree
			Chd->set_root(this->Root);
		}
	}

	void node::host_update(
		double 							aDeltaTime, 
		double 							aTime, 
		const std::vector<float>& 		aAnimationWeight, 
		const std::vector<animation>& 	aPlaybackAnimation,
		const std::vector<force>& 		aAppliedForces
	) {
		//tex:
		// It is the responsibility of the model class to insure that the sum of the contribution
		// factors (weights) is equal to 1.
		// $$ 1 = w^{b} + \sum_{\forall A \in Anim} w_{i} $$
		// $$ T = T^{base} \cdot w^{base} + \sum_{\forall i \in A} T_{i}^{A} \cdot w_{i}^{A} $$ 

		// Bind Pose Transform
		this->CurrentTransform = (this->DefaultTransform * aAnimationWeight[0]);

		// TODO: Figure out how to load animations per node. Also incredibly slow right now. Optimize Later.
		// Overrides/Averages Animation Transformations with Bind Pose Transform based on weights.
		for (size_t i = 0; i < aPlaybackAnimation.size(); i++) {
			// Check if Animation Data exists for this node, if not, use bind pose.
			// Pull animation for readability.
			auto& NodeAnimation = aPlaybackAnimation[i][this->Identifier];
			float Weight = aAnimationWeight[i + 1];
			if (NodeAnimation.exists()) {
				// Calculate time in ticks
				float TickerTime = aTime * aPlaybackAnimation[i].TicksPerSecond;
				// Ensure TickerTime is within the bounds of the animation.
				float BoundedTickerTime = std::fmod(TickerTime, aPlaybackAnimation[i].Stop - aPlaybackAnimation[i].Start) + aPlaybackAnimation[i].Start;
				if (this->Root == this) {
					this->CurrentTransform += this->DefaultTransform * NodeAnimation[BoundedTickerTime] * Weight;
				}
				else {
					this->CurrentTransform += NodeAnimation[BoundedTickerTime] * Weight;
				}
			}
			else {
				// Animation Data Does Not Exist, use bind pose animation.
				this->CurrentTransform += this->DefaultTransform * Weight;
			}
		}
	}

	void node::device_update(
		double 									aDeltaTime, 
		double 									aTime, 
		const std::vector<float>& 				aAnimationWeight, 
		const std::vector<phys::animation>& 	aPlaybackAnimation,
		const std::vector<phys::force>& 		aAppliedForces
	) {
		// Does nothing, by definition, agnostic of GPU module.
	}
	
}
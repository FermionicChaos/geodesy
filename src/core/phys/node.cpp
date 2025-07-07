#include <geodesy/core/phys/node.h>

#include <geodesy/core/gfx/node.h>

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
		this->Transformation 	= {
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
	math::mat<float, 4, 4> node::transform(const std::vector<float>& aAnimationWeight, const std::vector<phys::animation>& aPlaybackAnimation, double aTime) const {
		// This calculates the global transform of the node which it is
		// being called from. If there are no animations associated with 
		// the node hierarchy, the bind pose transformations will be used
		// instead. If there are animations associated with the node hierarchy,
		// Then the animation transformations will be used in a weighted average.

		//tex:
		// It is the responsibility of the model class to insure that the sum of the contribution
		// factors (weights) is equal to 1.
		// $$ 1 = w^{b} + \sum_{\forall A \in Anim} w_{i} $$
		// $$ T = T^{base} \cdot w^{base} + \sum_{\forall i \in A} T_{i}^{A} \cdot w_{i}^{A} $$ 
		//

		// Bind Pose Transform
		math::mat<float, 4, 4> NodeTransform = (this->Transformation * aAnimationWeight[0]);

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
					NodeTransform += this->Transformation * NodeAnimation[BoundedTickerTime] * Weight;
				}
				else {
					NodeTransform += NodeAnimation[BoundedTickerTime] * Weight;
				}
			}
			else {
				// Animation Data Does Not Exist, use bind pose animation.
				NodeTransform += this->Transformation * Weight;
			}
		}

		// Recursively apply parent transformations.
		if (this->Root != this) {
			return this->Parent->transform(aAnimationWeight, aPlaybackAnimation, aTime) * NodeTransform;
		}
		else {
			return NodeTransform;
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
		this->Transformation = aNode->Transformation;
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

	void node::update(
		double 							aDeltaTime, 
		double 							aTime, 
		const std::vector<float>& 		aAnimationWeight, 
		const std::vector<animation>& 	aPlaybackAnimation,
		const std::vector<force>& 		aAppliedForces
	) {
		// Go to child nodes and update children nodes.
		// for (auto Chd : this->Child) {
		// 	Chd->update(
		// 		aDeltaTime, 
		// 		aTime, 
		// 		aAnimationWeight, 
		// 		aPlaybackAnimation
		// 	);
		// }
		
		// //update_info UpdateInfo;
		// // Newtons First Law: An object in motion tends to stay in motion.
		// // Newtons Second Law: The change in momentum of an object is equal to the forces applied to it.
		// // Newtons Third Law: For every action, there is an equal and opposite reaction.
		// math::mat<float, 3, 3> InvertedInertiaTensor = math::inverse(this->InertiaTensor);

		// // How the momentum of the object will change when a force is applied to it.
		// this->AngularMomentum += aAppliedTorque * aDeltaTime;

		// // How the object will move according to its current momentum.
		// this->LinearMomentum += (aAppliedForce + this->InputForce) * aDeltaTime;
		// this->Position += (this->LinearMomentum / this->Mass + this->InputVelocity) * aDeltaTime;
	}
	
}
#include <geodesy/core/phys/node.h>

namespace geodesy::core::phys {

	// This function deep copies the other node tree.
	void node::copy(const node* aInput) {

	}

	// This function is used to swap the contents of two nodes.
	void node::swap(node* aInput) {

	}

	void node::update(float aDeltaTime) {
		// The actual physics done on the node based on applied forces. (After Collision Detection, and response forces).

		// Update Linear & Angular Momentum.


		// Update the node's position, rotation, and scale.
		this->Position += (this->LinearMomentum / this->Mass) * aDeltaTime;

		// Invert the inertia tensor to get the angular velocity.
		math::vec<float, 3> AngularVelocity = this->AngularMomentum;// * math::inverse(this->InertiaTensor);

		// Update the orientation using quaternion rotation.
		//this->Orientation = math::quaternion<float>::rotate(this->Orientation, AngularVelocity, aDeltaTime);

		// Calculate final node transformation matrix.
		//this->Orientation *= math::rotation(math::length(AngularVelocity) * aDeltaTime, AngularVelocity);
	}
	
}
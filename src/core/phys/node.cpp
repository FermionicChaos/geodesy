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
    }
	
}
#include <geodesy/core/hid/mouse.h>

#include <string.h>

namespace geodesy::core::hid {

	mouse::mouse() {
		memset(this->State, 0, sizeof(this->State));
	}

	mouse::~mouse() {}

	mouse::state& mouse::operator[](int aButtonID) {
		return this->State[aButtonID];
	}

	mouse::state mouse::operator[](int aButtonID) const {
		return this->State[aButtonID];
	}

}

#include <geodesy/core/hid/mouse.h>

#include <string.h>

namespace geodesy::core::hid {

	mouse::mouse() {
		memset(this->State, 0, sizeof(this->State));
		this->Scroll = { 0.0f, 0.0f };
		this->Time = 0.0f;
		this->Position = { 0.0f, 0.0f };
		this->Velocity = { 0.0f, 0.0f };
		this->Acceleration = { 0.0f, 0.0f };
	}

	mouse::~mouse() {}

	mouse::state& mouse::operator[](int aButtonID) {
		return this->State[aButtonID];
	}

	mouse::state mouse::operator[](int aButtonID) const {
		return this->State[aButtonID];
	}

	void mouse::update(double aDeltaTime) {
		if (this->NewPosition) {
			// If new position, set false for next frame.
			this->NewPosition = false;
		} else {
			this->Velocity = { 0.0f, 0.0f };
			this->Acceleration = { 0.0f, 0.0f };
		}
	}

}

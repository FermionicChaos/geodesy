#include <geodesy/core/hid/keyboard.h>

#include <string.h>

namespace geodesy::core::hid {

	keyboard::keyboard() {
		memset(this->Key, 0, sizeof(this->Key));
	}

	keyboard::~keyboard() {}

	keyboard::key& keyboard::operator[](int aKeyID) {
		return this->Key[aKeyID];
	}

	keyboard::key keyboard::operator[](int aKeyID) const {
		return this->Key[aKeyID];
	}

}

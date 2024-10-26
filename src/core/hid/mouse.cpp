#include <geodesy/core/hid/mouse.h>

#include <geodesy/core/math.h>

namespace geodesy {
	namespace core {
		namespace hid {

			mouse::mouse() {
				Mode = MODE_NORMAL;
			}

			mouse::~mouse() {

			}

			bool mouse::operator[](int buttonID) {
				if ((Action[buttonID] == BUTTON_PRESS) || (Action[buttonID] == BUTTON_REPEAT)) {
					return true;
				}
				else {
					return false;
				}
			}

		}
	}
}

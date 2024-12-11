#pragma once
#ifndef GEODESY_CORE_HID_MOUSE_H
#define GEODESY_CORE_HID_MOUSE_H

#include "../math.h"

namespace geodesy::core::hid {

	class mouse {
	public:

		enum button {
			BUTTON_LEFT		= 0,
			BUTTON_RIGHT	= 1,
			BUTTON_MIDDLE	= 2,
			BUTTON_4		= 3,
			BUTTON_5		= 4,
			BUTTON_6		= 5,
			BUTTON_7		= 6,
			BUTTON_LAST		= 7
		};
		enum action {
			BUTTON_RELEASE	= 0,
			BUTTON_PRESS	= 1,
			BUTTON_REPEAT	= 2
		};

		enum mode {
			MODE_NORMAL		= 0x00034001,
			MODE_HIDDEN		= 0x00034002,
			MODE_DISABLED	= 0x00034003
		};

		struct state {
			int Action;
			int Modifier;
		};

		state State[8];
		math::vec<float, 2> Scroll;
		math::vec<float, 2> Position;
		math::vec<float, 2> Velocity;
		math::vec<float, 2> Acceleration;

		mouse();
		~mouse();

		state& operator[](int aButtonID);
		state operator[](int aButtonID) const;

	};

}

#endif // !GEODESY_CORE_HID_MOUSE_H
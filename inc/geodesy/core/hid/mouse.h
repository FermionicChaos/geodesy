#ifndef MOUSE_H
#define MOUSE_H

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

		// Mouse Input
		int Action[8];
		int Modifier[8];
		math::vec<float, 2> Scroll;

		// Cursor Variables
		int Mode;
		bool InWindow;
		math::vec<float, 2> Position;
		math::vec<float, 2> Velocity;
		math::vec<float, 2> Acceleration;

		mouse();
		~mouse();

		bool operator[](int buttonID);

	};

}

#endif // !MOUSE_H
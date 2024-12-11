#pragma once
#ifndef GEODESY_CORE_HID_INPUT_H
#define GEODESY_CORE_HID_INPUT_H

#include "config.h"
#include "keyboard.h"
#include "mouse.h"
#include "joystick.h"

namespace geodesy::core::hid {

	// This is a generalized user input class, which forwards input
	// to user objects. The members of this class are highly platform
	// dependent.

	class input {
	public:

		enum type {
			KEYBOARD,
			MOUSE,
			JOYSTICK
		};

		type 			Event;
		keyboard 		Keyboard;
		mouse 			Mouse;
		// joystick Joystick;
		
	};
	
}

#endif // !GEODESY_CORE_HID_INPUT_H
#ifndef GEODESY_BLTN_OBJ_SYSTEM_DISPLAY_H
#define GEODESY_BLTN_OBJ_SYSTEM_DISPLAY_H

#include <geodesy/engine.h>

struct GLFWmonitor;
struct GLFWvidmode;

namespace geodesy::bltn::obj {
	
	class system_display {
	public:

		static void get_system_displays(engine* aEngine);

		GLFWmonitor* Monitor;

		//VkDisplayKHR Handle;

		system_display(GLFWmonitor* aMonitor);

	};

}

#endif // !GEODESY_BLTN_OBJ_SYSTEM_DISPLAY_H
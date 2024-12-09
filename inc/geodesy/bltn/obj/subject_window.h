#pragma once
#ifndef GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H
#define GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H

#include <geodesy/engine.h>

/*
subject_window is a class that is intended to use any existing render target and cast it into a window in another
stage. Itself is not a render target (subject), and cannot be drawn to. It is merely a reflector class for another
render target. 

For example, in scene3d, the camera3d render target has a series of images that are rendered to, and that needs to
be referenced by a system_window class for presentation.
*/

namespace geodesy::bltn::obj {

	class subject_window : public ecs::object {
	public:

		subject_window(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, std::string aName, std::shared_ptr<ecs::subject> aSubject);

	};

}

#endif // !GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H
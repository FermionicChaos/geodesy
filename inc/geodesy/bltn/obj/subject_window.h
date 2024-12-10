#pragma once
#ifndef GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H
#define GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H

#include <geodesy/engine.h>

/*
subject_window itself is a misnomer class. It is not a subject (render target) itself, it is merely a class
that forwards the outputs from other subjects (render targets). It is a window that cannot be drawn to. It is a
window in a sense that it views the contents of another subject. This object bridges the render outputs between
stages and will be useful to creating complex render systems between stages.

Example:
In scene3d, the camera3d render target has a series of images that are rendered to, and that needs to
be referenced by a system_window class for presentation, with or without GUI elements.
*/

namespace geodesy::bltn::obj {

	class subject_window : public ecs::object {
	public:

		core::math::vec<float, 2> Size;
		std::shared_ptr<ecs::subject> SubjectSource; 		// ! Change to std::weak_ptr

		subject_window(
			std::shared_ptr<core::gcl::context> aContext, 
			ecs::stage* aStage, 
			std::string aName, 
			std::shared_ptr<ecs::subject> aSubjectSource, 
			core::math::vec<float, 2> aSize,
			core::math::vec<float, 3> aPosition = { 0.0f, 0.0f, 0.5f },
			core::math::vec<float, 2> aDirection = { 90.0f, 270.0f }
		);

		std::vector<core::gfx::draw_call> draw(ecs::subject* aSubject) override;

	private:

		std::map<ecs::subject*, std::vector<std::vector<std::vector<core::gfx::draw_call>>>> OverridenRenderer;

		std::vector<std::vector<std::vector<core::gfx::draw_call>>> specialized_renderer(ecs::subject* aSubjectTarget);

	};

}

#endif // !GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H
#pragma once
#ifndef GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H
#define GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H

#include <geodesy/engine.h>

#include "window.h"

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

		struct forward_draw_call : ecs::object::draw_call {
			forward_draw_call(
				object* 							aObject, 
				core::gfx::mesh::instance* 			aMeshInstance,
				ecs::subject* 						aSubjectSource,
				size_t 								aSourceFrameIndex,
				window* 							aSubjectTarget,
				size_t 								aTargetFrameIndex
			);
		};

		struct forward_renderer : ecs::object::renderer {
			std::vector<std::vector<std::vector<std::shared_ptr<draw_call>>>> OverridenDrawCallList;
			forward_renderer(
				ecs::object* 						aObject, 
				ecs::subject* 						aSubjectSource,
				ecs::subject* 						aSubjectTarget
			);
			~forward_renderer() override;
		};

		struct creator : object::creator {
			std::shared_ptr<ecs::subject> 			Subject;
			creator();
		};

		core::math::vec<float, 2> 			Size;
		std::shared_ptr<ecs::subject> 		SubjectSource; 		// ! Change to std::weak_ptr

		subject_window(std::shared_ptr<core::gpu::context> aContext, ecs::stage* aStage, creator* aSubjectWindowCreator);

		std::vector<std::shared_ptr<draw_call>> draw(ecs::subject* aSubject) override;

	};	

}

#endif // !GEODESY_BLTN_OBJ_SUBJECT_WINDOW_H
#pragma once
#ifndef GEODESY_BLTN_OBJ_WINDOW_H
#define GEODESY_BLTN_OBJ_WINDOW_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class window : public runtime::subject {
	public:

		struct window_draw_call : object::draw_call {
			window_draw_call(
				object* 								aObject, 
				core::gfx::mesh::instance* 				aMeshInstance,
				window* 								aWindow,
				size_t 									aFrameIndex
			);
		};

		struct window_renderer : runtime::object::renderer {
			window_renderer(runtime::object* aObject, window* aWindow);
		};

		struct creator : subject::creator {
			core::gpu::image::format 	PixelFormat;
			bool 						Resizable;		
			bool 						Decorated;		
			bool 						UserFocused;	
			bool 						AutoMinimize;	
			bool 						Floating;		
			bool 						Maximized;		
			bool 						Minimized;		
			bool 						Visible;		
			bool 						ScaleToMonitor;	
			bool 						CenterCursor;	
			bool 						FocusOnShow;	
			bool 						Transparency;	
			bool 						Fullscreen;		
			bool 						Hovered;		
			bool 						ShouldClose;
			creator();
		};

		std::shared_ptr<core::gpu::buffer> WindowUniformBuffer;

		window(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aWindowCreator);

		std::shared_ptr<renderer> default_renderer(runtime::object* aObject) override;
		
	};

}

#endif // !GEODESY_BLTN_WINDOW_H
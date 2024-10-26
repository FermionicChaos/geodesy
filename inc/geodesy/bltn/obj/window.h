#pragma once
#ifndef GEODESY_BLTN_OBJ_WINDOW_H
#define GEODESY_BLTN_OBJ_WINDOW_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class window : public ecs::subject {
	public:

		struct property {
			bool Resizable;		
			bool Decorated;		
			bool UserFocused;	
			bool AutoMinimize;	
			bool Floating;		
			bool Maximized;		
			bool Minimized;		
			bool Visible;		
			bool ScaleToMonitor;	
			bool CenterCursor;	
			bool FocusOnShow;	
			bool Transparency;	
			bool Fullscreen;		
			bool Hovered;		
			bool ShouldClose;
			property();
		};

		std::shared_ptr<core::gcl::pipeline::rasterizer> Rasterizer;

		window(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, std::string aName, core::math::vec<uint, 3> aFrameResolution, double aFrameRate, uint32_t aFrameCount, uint32_t aAttachmentCount);

		std::shared_ptr<core::gfx::renderer> make_default_renderer(ecs::object* aObject) override;
		
	};

}

#endif // !GEODESY_BLTN_WINDOW_H
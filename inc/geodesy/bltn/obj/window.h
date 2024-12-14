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

		std::shared_ptr<core::gcl::buffer> WindowUniformBuffer;

		window(
			std::shared_ptr<core::gcl::context> 	aContext, 
			ecs::stage* 							aStage, 
			std::string 							aName, 
			core::gcl::image::format 				aFormat,
			core::math::vec<uint, 3> 				aResolution,
			core::math::vec<float, 3> 				aPosition = { 0.0f, 0.0f, 0.0f },
			core::math::vec<float, 2> 				aDirection = { 0.0f, 0.0f }
		);

		std::vector<std::vector<core::gfx::draw_call>> default_renderer(ecs::object* aObject) override;
		
	};

}

#endif // !GEODESY_BLTN_WINDOW_H
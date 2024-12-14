#pragma once
#ifndef GEODESY_BLTN_STG_CANVAS_H
#define GEODESY_BLTN_STG_CANVAS_H

#include <geodesy/engine.h>

#include "../obj/window.h"

namespace geodesy::bltn::stg {

	class canvas : public ecs::stage {
	public:

		canvas(std::shared_ptr<core::gcl::context> aContext, std::string aName, std::shared_ptr<obj::window> aWindow);

	};	

}

#endif // !GEODESY_BLTN_STG_CANVAS_H
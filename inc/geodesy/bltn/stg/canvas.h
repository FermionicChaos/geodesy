#pragma once
#ifndef GEODESY_BLTN_STG_CANVAS_H
#define GEODESY_BLTN_STG_CANVAS_H

#include <geodesy/engine.h>

#include "../obj/window.h"

namespace geodesy::bltn::stg {

	class canvas : public ecs::stage {
	public:

		std::vector<std::shared_ptr<bltn::obj::window>> Window;

		canvas(std::shared_ptr<core::gcl::context> aContext, std::string aName);

		virtual ecs::subject::render_info render() override;

	};	

}

#endif // !GEODESY_BLTN_STG_CANVAS_H
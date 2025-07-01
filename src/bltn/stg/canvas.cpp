#include <geodesy/bltn/stg/canvas.h>

#include <iostream>

namespace geodesy::bltn::stg {

	canvas::canvas(std::shared_ptr<core::gpu::context> aContext, std::string aName, std::shared_ptr<obj::window> aWindow) : runtime::stage(aContext, aName) {
		this->Object.push_back(aWindow);
	}

}

#pragma once
#ifndef GEODESY_BLTN_STG_SCENE3D_H
#define GEODESY_BLTN_STG_SCENE3D_H

#include <geodesy/engine.h>

#include "../obj.h"

namespace geodesy::bltn::stg {

	class scene3d : public runtime::stage {
	public:

		scene3d(std::shared_ptr<core::gpu::context> aContext, std::string aName);

	};	

}

#endif // !GEODESY_BLTN_STG_SCENE3D_H
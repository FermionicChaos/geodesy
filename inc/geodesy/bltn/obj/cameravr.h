#pragma once
#ifndef GEODESY_BLTN_OBJ_CAMERAVR_H
#define GEODESY_BLTN_OBJ_CAMERAVR_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class cameravr : public ecs::subject {

		struct creator : public ecs::subject::creator {
			/*
			Image Usage
			Image Format
			Color Space
			*/
		};

		class geometry_buffer : public framechain {

		};

		cameravr(std::shared_ptr<core::gpu::context> aContext, ecs::stage* aStage, creator* aCreator);

	};

}

#endif // !GEODESY_BLTN_OBJ_CAMERAVR_H
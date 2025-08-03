#pragma once
#ifndef GEODESY_BLTN_OBJ_CAMERAVR_H
#define GEODESY_BLTN_OBJ_CAMERAVR_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class cameravr : public runtime::subject {
	public:

		struct creator : public runtime::subject::creator {
			/*
			Image Usage
			Image Format
			Color Space
			*/
			creator();
		};

		// Runtime Type Information (RTTI) ID for the cameravr class.
		constexpr static uint32_t rttiid = geodesy::runtime::generate_rttiid<cameravr>();

		class geometry_buffer : public framechain {

		};

		cameravr(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aCreator);

	};

}

#endif // !GEODESY_BLTN_OBJ_CAMERAVR_H
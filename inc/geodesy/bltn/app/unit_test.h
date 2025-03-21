#pragma once
#ifndef GEODESY_BLTN_APP_UNIT_TEST_H
#define GEODESY_BLTN_APP_UNIT_TEST_H

#include <geodesy/engine.h>

#include "../obj.h"
#include "../stg.h"

namespace geodesy::bltn {

	class unit_test : public ecs::app {
	public:

		std::shared_ptr<core::gpu::context> 		DeviceContext; 		// Device Context which will be used for all gfx and computation operations.
		std::shared_ptr<bltn::obj::system_window>	Window; 			// Main Application Window.

		unit_test(engine* aEngine);
		~unit_test();

		void run() override;

		void math_test();

	};

}

#endif // GEODESY_BLTN_APP_UNIT_TEST_H
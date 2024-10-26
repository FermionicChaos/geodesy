#pragma once
#ifndef GEODESY_CORE_APP_H
#define GEODESY_CORE_APP_H

#include "../config.h"
#include "../core/gcl/config.h"
#include "../core/math.h"
#include "stage.h"

namespace geodesy::ecs {

	class app {
	public:

		std::mutex								Mutex;
		engine*									Engine;
		std::string								Name;
		core::math::vec<uint, 3>				Version;
		double									TimeStep;
		double									Time;
		std::vector<std::shared_ptr<stage>>		Stage;
		
		app(engine* aEngine, std::string aName, core::math::vec<uint, 3> aVersion);

		void init();
		virtual void run() = 0;
		
		std::map<std::shared_ptr<core::gcl::context>, object::update_info> update(double aDeltaTime);
		std::map<std::shared_ptr<core::gcl::context>, subject::render_info> render();

	};

}

#endif // !GEODESY_CORE_APP_H


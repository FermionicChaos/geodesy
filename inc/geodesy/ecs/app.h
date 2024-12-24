#pragma once
#ifndef GEODESY_CORE_APP_H
#define GEODESY_CORE_APP_H

#include "../config.h"

#include "../core/io.h"
#include "../core/math.h"
#include "../core/util.h"
#include "../core/lgc.h"
#include "../core/phys.h"
#include "../core/hid.h"
#include "../core/gcl.h"
#include "../core/gfx.h"
#include "../core/sfx.h"

#include "object.h"
#include "subject.h"
#include "stage.h"

namespace geodesy::ecs {

	class app {
	public:

		std::mutex											Mutex;
		engine*												Engine;
		std::string											Name;
		core::math::vec<uint, 3>							Version;
		double												TimeStep;
		double												Time;
		std::vector<std::shared_ptr<stage>>					Stage;
		std::map<std::string, std::shared_ptr<stage>> 		StageLookup;
		
		app(engine* aEngine, std::string aName, core::math::vec<uint, 3> aVersion);

		template<typename T, typename... Args>
		std::shared_ptr<T> create_stage(std::shared_ptr<core::gcl::context> aContext, std::string aName, Args&&... aArgs) {
			std::shared_ptr<T> NewStage(new T(
				aContext,
				std::move(aName),
				std::forward<Args>(aArgs)...
			));
			this->Stage.push_back(NewStage);
			this->StageLookup[NewStage->Name] = NewStage;
			return NewStage;
		}

		void init();
		virtual void run() = 0;
		
		std::map<std::shared_ptr<core::gcl::context>, core::gcl::submission_batch> update(double aDeltaTime);
		std::map<std::shared_ptr<core::gcl::context>, core::gcl::submission_batch> render();

	};

}

#endif // !GEODESY_CORE_APP_H


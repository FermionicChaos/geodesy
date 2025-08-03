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
#include "../core/gpu.h"
#include "../core/gfx.h"
#include "../core/sfx.h"

#include "object.h"
#include "subject.h"
#include "stage.h"

namespace geodesy::runtime {

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
		~app();

		// Polymorphic factory method for runtime object creation using type information
		template<typename T>
		static std::shared_ptr<T> create(std::shared_ptr<core::gpu::context> aContext, runtime::stage::creator* aCreator) {
			std::shared_ptr<T> NewObject = geodesy::make<T>(
				aContext,
				static_cast<typename T::creator*>(aCreator)
			);
			return NewObject;
		}

		void init();
		virtual void run() = 0;
		
		void update(double aDeltaTime);
		std::map<std::shared_ptr<core::gpu::context>, core::gpu::submission_batch> render();

		virtual std::shared_ptr<stage> build_stage(std::shared_ptr<core::gpu::context> aContext, stage::creator* aStageCreator);

	};

}

#endif // !GEODESY_CORE_APP_H


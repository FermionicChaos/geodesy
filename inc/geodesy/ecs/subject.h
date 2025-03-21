#pragma once
#ifndef GEODESY_CORE_SUBJECT_H
#define GEODESY_CORE_SUBJECT_H

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

namespace geodesy::ecs {
	
	class subject : public object {
	public:

		struct creator : object::creator {
			core::math::vec<uint, 3> 		Resolution;
			uint 							FrameCount;
			float 							FrameRate;
			int 							ImageUsage;
			creator();
		};

		std::shared_ptr<core::gpu::framechain> 						Framechain;
		std::shared_ptr<core::gpu::pipeline> 						Pipeline;
		std::shared_ptr<core::gpu::command_pool>					CommandPool;
		std::shared_ptr<core::gpu::semaphore_pool> 					SemaphorePool;
		std::vector<core::gpu::command_batch>						RenderingOperations;
		VkSemaphore 												NextFrameSemaphore;
		VkSemaphore 												PresentFrameSemaphore;

		subject(std::shared_ptr<core::gpu::context> aContext, stage* aStage, creator* aSubjectCreator);
		~subject();

		virtual bool is_subject() override;
		virtual std::shared_ptr<renderer> default_renderer(object* aObject);
		virtual core::gpu::submission_batch render(stage* aStage);

	};

	

}

#endif // GEODESY_CORE_SUBJECT_H
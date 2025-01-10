#pragma once
#ifndef GEODESY_CORE_GCL_FRAMEBUFFER_H
#define GEODESY_CORE_GCL_FRAMEBUFFER_H

#include "../../config.h"
#include "config.h"
#include "image.h"
#include "pipeline.h"

namespace geodesy::core::gcl {

	// This class is used to interface actual resource attachements to a pipeline. Similar to descriptor sets.
	class framebuffer {
	public:

		std::vector<VkClearValue> 	ClearValue;

		std::shared_ptr<context> 	Context;
		VkFramebuffer 				Handle;

		framebuffer(std::shared_ptr<context> aContext, std::shared_ptr<pipeline> aPipeline, std::vector<std::shared_ptr<image>> aImageAttachements, math::vec<uint, 3> aResolution);
		framebuffer(std::shared_ptr<context> aContext, std::shared_ptr<pipeline> aPipeline, std::map<std::string, std::shared_ptr<image>> aImage, std::vector<std::string> aAttachmentSelection, math::vec<uint, 3> aResolution);
		~framebuffer();
		
	};

}

#endif // !GEODESY_CORE_GCL_FRAMEBUFFER_H
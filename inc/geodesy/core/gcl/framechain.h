#pragma once
#ifndef GEODESY_CORE_GCL_FRAMECHAIN_H
#define GEODESY_CORE_GCL_FRAMECHAIN_H

#include "../../config.h"
#include "../math.h"
#include "../lgc/timer.h"
#include "config.h"
#include "image.h"

namespace geodesy::core::gcl {

	class framechain {
	public:

		math::vec<uint, 3>												Resolution;
		uint32_t														ReadIndex;
		uint32_t														DrawIndex;
		double															FrameRate;
		core::lgc::timer 												Timer;
		std::shared_ptr<context> 										Context;
		std::vector<std::map<std::string, std::shared_ptr<image>>> 		Image;

		framechain(std::shared_ptr<context> aContext, double aFrameRate, uint32_t aFrameCount);
		virtual ~framechain() = default;

	}; 	

}

#endif // !GEODESY_CORE_GCL_FRAMECHAIN_H
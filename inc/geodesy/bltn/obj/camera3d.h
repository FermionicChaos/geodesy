#pragma once
#ifndef GEODESY_BLTN_OBJ_CAMERA3D_H
#define GEODESY_BLTN_OBJ_CAMERA3D_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class camera3d : public ecs::subject {
	public:

		class geometry_buffer : public core::gcl::framechain {
		public:
			geometry_buffer(std::shared_ptr<core::gcl::context> aContext, core::math::vec<uint, 3> aResolution, double aFrameRate, size_t aFrameCount);
		};

		core::math::mat<float, 4, 4> Projection;

		camera3d(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, std::string aName, core::math::vec<uint, 3> aFrameResolution, double aFrameRate, uint32_t aFrameCount);
		~camera3d();

		std::vector<std::vector<core::gfx::draw_call>> default_renderer(ecs::object* aObject) override;

	};	

}

#endif // !GEODESY_BLTN_OBJ_CAMERA3D_H
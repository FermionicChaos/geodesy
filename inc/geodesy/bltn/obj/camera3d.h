#pragma once
#ifndef GEODESY_BLTN_OBJ_CAMERA3D_H
#define GEODESY_BLTN_OBJ_CAMERA3D_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class camera3d : public ecs::subject {
	public:

		struct creator : subject::creator {
			float FOV;
			float Near;
			float Far;
		};

		float FOV, Near, Far;
		std::shared_ptr<core::gcl::buffer> 	CameraUniformBuffer;

		camera3d(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, creator* aCamera3DCreator);
		~camera3d();

		// Implement input
		void input(const core::hid::input& aInput) override;
		void update(double aDeltaTime, core::math::vec<float, 3> aAppliedForce = { 0.0f, 0.0f, 0.0f }, core::math::vec<float, 3> aAppliedTorque = { 0.0f, 0.0f, 0.0f }) override;
		std::vector<std::vector<core::gfx::draw_call>> default_renderer(ecs::object* aObject) override;

	};	

}

#endif // !GEODESY_BLTN_OBJ_CAMERA3D_H
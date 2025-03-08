#pragma once
#ifndef GEODESY_BLTN_OBJ_CAMERA3D_H
#define GEODESY_BLTN_OBJ_CAMERA3D_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class camera3d : public ecs::subject {
	public:

		class geometry_buffer : public core::gcl::framechain {
		public:
			/*
			The Geometry Buffer contains the Following Attachments.
			// --- Final Output --- //
			Color: 		The Final Output color of the camera after rendering the scene plus post processing.
			// --- Classic Opaque Geometry Buffer --- //
			Albedo: 	The Color of the rasterized scene of all opaque and transparent objects.
			Position: 	The Fragment Position of the rasterized scene of all opaque and transparent objects.
			Normal: 	The Fragment Normal of the rasterized scene of all opaque and transparent objects.
			Emissive: 	The Emissive Color of the rasterized scene of all opaque and transparent objects.
			SS: 		The Specular & Shininess values of the rasterized scene of all opaque and transparent objects.
			ORM: 		The Occlusion, Roughness, and Metallic values of the rasterized scene of all opaque and transparent objects.
			Depth: 		The Depth of the rasterized scene of all opaque and transparent objects.
			// --- Ray Traced Deferred Shading Output --- //
			// --- Translucent Ray Tracing Layer --- //
			*/
			geometry_buffer(std::shared_ptr<core::gcl::context> aContext, core::math::vec<uint, 3> aResolution, double aFrameRate, size_t aFrameCount);
		};

		struct deferred_draw_call : object::draw_call {
			deferred_draw_call(
				object* 								aObject, 
				core::gfx::mesh::instance* 				aMeshInstance,
				camera3d* 								aCamera3D,
				size_t 									aFrameIndex
			);
		};

		struct deferred_renderer : object::renderer {
			deferred_renderer(
				object* aObject, 
				camera3d* aCamera3D
			);
		};

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
		std::shared_ptr<renderer> default_renderer(ecs::object* aObject) override;

	};

}

#endif // !GEODESY_BLTN_OBJ_CAMERA3D_H
#pragma once
#ifndef GEODESY_BLTN_OBJ_CAMERA3D_H
#define GEODESY_BLTN_OBJ_CAMERA3D_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class camera3d : public runtime::subject {
	public:

		class geometry_buffer : public framechain {
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
			geometry_buffer(std::shared_ptr<core::gpu::context> aContext, core::math::vec<uint, 3> aResolution, double aFrameRate, size_t aFrameCount);
		};

		struct rasterizer_draw_call : object::draw_call {
			rasterizer_draw_call(
				camera3d* 	aCamera3D,
				size_t 		aFrameIndex,
				object* 	aObject, 
				size_t 		aMeshInstanceIndex
			);
			void update(
				subject* 	aSubject, 
				size_t 		aFrameIndex,
				object* 	aObject, 
				size_t 		aMeshInstanceIndex
			) override;
		};

		struct ray_trace_draw_call : object::draw_call {
			ray_trace_draw_call(
				camera3d* 			aCamera3D,
				size_t 				aFrameIndex,
				runtime::stage* 	aStage
			);
			void update(
				subject* 			aSubject, 
				size_t 				aFrameIndex,
				runtime::stage* 	aStage
			);
		};

		// This is the default rasterizer for camera3d.
		struct stage_rasterizer : object::renderer {
			stage_rasterizer(
				camera3d* aCamera3D,
				object* aObject
			);
		};

		// This is the hybrid ray 
		struct stage_postprocessor : object::renderer {
			stage_postprocessor(
				camera3d* aCamera3D,
				runtime::stage* aStage
			);
			virtual void update(
				double aDeltaTime = 0.0f, 
				double aTime = 0.0f
			) override;
		};

		struct creator : subject::creator {
			float FOV;
			float Near;
			float Far;
			creator();
		};

		// Runtime Type Information (RTTI) ID for the camera3d class.
		constexpr static uint32_t rttiid = geodesy::runtime::generate_rttiid<camera3d>();

		float FOV, Near, Far;

		camera3d(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aCamera3DCreator);
		~camera3d();

		// Implement input
		void input(const core::hid::input& aInput) override;
		void host_update(
			double 										aDeltaTime = 0.0f, 
			double 										aTime = 0.0f,
			const std::vector<core::phys::force>& 		aAppliedForces = {}
		) override;
		virtual void device_update(
			double 										aDeltaTime = 0.0f, 
			double 										aTime = 0.0f, 
			const std::vector<core::phys::force>& 		aAppliedForces = {}
		) override;
		std::shared_ptr<renderer> default_renderer(runtime::object* aObject) override;
		std::shared_ptr<renderer> default_post_processor(runtime::stage* aStage) override;
		core::gpu::submission_batch render(runtime::stage* aStage) override;

		// These will create the pipelines for the camera3d.
		std::shared_ptr<core::gpu::pipeline> create_opaque_rasterizing_pipeline(creator* aCamera3DCreator);
		// std::shared_ptr<core::gpu::pipeline> create_translucent_rasterizing_pipeline(creator* aCamera3DCreator);
		std::shared_ptr<core::gpu::pipeline> create_opaque_ray_tracing_pipeline(creator* aCamera3DCreator);
		// std::shared_ptr<core::gpu::pipeline> create_translucent_ray_tracing_pipeline(creator* aCamera3DCreator);
		// std::shared_ptr<core::gpu::pipeline> create_final_compositor_pipeline(creator* aCamera3DCreator);	

	};

}

#endif // !GEODESY_BLTN_OBJ_CAMERA3D_H
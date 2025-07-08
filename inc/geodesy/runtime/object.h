#pragma once
#ifndef GEODESY_CORE_OBJECT_H
#define GEODESY_CORE_OBJECT_H

#include "../config.h"
#include "../core/io.h"
#include "../core/math.h"
#include "../core/phys.h"
#include "../core/util.h"
#include "../core/lgc.h"
#include "../core/hid.h"
#include "../core/gpu.h"
#include "../core/gfx.h"
#include "../core/sfx.h"

namespace geodesy::runtime {
	
	class object : public core::gfx::node {
	public:

		struct uniform_data {
			alignas(16) core::math::vec<float, 3> 					Position;
			alignas(16) core::math::mat<float, 4, 4> 				Orientation;
			alignas(16) core::math::vec<float, 3> 					Scale;
			uniform_data(
				core::math::vec<float, 3> aPosition, 
				core::math::vec<float, 3> aDirRight, 
				core::math::vec<float, 3> aDirUp, 
				core::math::vec<float, 3> aDirForward,
				core::math::vec<float, 3> aScale
			);
		};

		// A draw call represents a singular draw call for a single mesh instance in
		// the node hiearchy of the model. Distance from the camera is determined
		struct draw_call {
			core::gfx::material::transparency 						TransparencyMode;
			float 													RenderingPriority;
			std::shared_ptr<core::gpu::context> 					Context;
			std::shared_ptr<core::gpu::framebuffer> 				Framebuffer;
			std::shared_ptr<core::gpu::descriptor::array> 			DescriptorArray;
			VkCommandBuffer 										DrawCommand;
			draw_call();
			virtual void update(
				subject* aSubject, 
				size_t aFrameIndex,
				object* aObject, 
				size_t aMeshInstanceIndex
			);
		};

		struct renderer {

			// Object & Subject must share the same device context.
			object* 												Object;
			subject* 												Subject;
			std::vector<std::vector<std::shared_ptr<draw_call>>> 	DrawCallList;

			renderer();
			renderer(object* aObject, subject* aSubject);
			virtual ~renderer();

			std::vector<std::shared_ptr<draw_call>> operator[](size_t aIndex) const;

			virtual void update(
				double aDeltaTime = 0.0f, 
				double aTime = 0.0f
			);

		};

		struct creator {
			std::string 					Name;
			std::string 					ModelPath;
			core::math::vec<float, 3> 		Position;
			core::math::vec<float, 2> 		Direction;
			core::math::vec<float, 3> 		Scale;
			std::vector<float> 				AnimationWeights;
			motion 							MotionType;
			bool 							GravityEnabled;
			bool 							CollisionEnabled;
			creator();
		};

		// ! ----- Host Data ----- ! //
		// ^ This data exists in Host memory.

		// * Object Metadata
		stage*																		Stage;
		engine*																		Engine;
		std::mutex																	Mutex;

		// * Object Input and Physics
		std::string																	Name;				// Name of the object.
		float 																		Theta, Phi;			// Radians			[rad]
		core::math::vec<float, 3>													DirectionRight;		// Right			[Normalized]
		core::math::vec<float, 3>													DirectionUp;		// Up				[Normalized]
		core::math::vec<float, 3>													DirectionFront;		// Backward			[Normalized]
		std::vector<float> 															AnimationWeights;
		std::vector<std::shared_ptr<core::io::file>> 								Asset;

		// ! ----- Device Data ----- ! //
		// ^ This is the data that exists on the GPU.
		std::shared_ptr<core::gfx::model>											Model;
		std::vector<core::phys::node*> 												LinearizedNodeTree;
		std::vector<core::gfx::mesh::instance*> 									TotalMeshInstance;
		std::shared_ptr<core::gpu::buffer> 											UniformBuffer;
		std::map<subject*, std::shared_ptr<renderer>>								Renderer;

		object(std::shared_ptr<core::gpu::context> aContext, stage* aStage, creator* aCreator);
		~object();

		void copy_data(const core::phys::node* aNode) override;

		virtual bool is_subject();
		virtual void input(const core::hid::input& aInput);
		virtual void update(
			double 										aDeltaTime = 0.0f, 
			double 										aTime = 0.0f, 
			const std::vector<float>& 					aAnimationWeight = { 1.0f }, 
			const std::vector<core::phys::animation>& 	aPlaybackAnimation = {},
			const std::vector<core::phys::force>& 		aAppliedForces = {}
		) override;
		virtual std::vector<std::shared_ptr<draw_call>> draw(subject* aSubject);

	protected:

		core::math::vec<float, 3> InputVelocity;
		core::math::vec<float, 3> InputForce;

	};

	std::vector<VkCommandBuffer> convert(std::vector<std::shared_ptr<object::draw_call>> aDrawCallList);

}

#endif // GEODESY_CORE_OBJECT_H
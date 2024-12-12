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
#include "../core/gcl.h"
#include "../core/gfx.h"
#include "../core/sfx.h"

namespace geodesy::ecs {
	
	class object /* : public std::enable_shared_from_this<object> */ {
	public:

		struct uniform_data {
			alignas(16) core::math::vec<float, 3> Position;
			alignas(16) core::math::mat<float, 4, 4> Orientation;
		};

		enum motion {
			STATIC,			// Object doesn't move.
			DYNAMIC,		// Object
			ANIMATED,
		};

		// ! ----- Host Data ----- ! //
		// ^ This data exists in Host memory.

		// * Object Metadata
		std::mutex																	Mutex;
		engine*																		Engine;
		stage*																		Stage;
		std::string																	Name;

		// * Object Input and Physics
		float																		Mass;				// Kilogram			[kg]
		float																		Time;				// Second 			[s]
		core::math::vec<float, 3>													Position;			// Meter			[m]
		float 																		Theta, Phi;			// Radians			[rad]
		core::math::vec<float, 3>													DirectionRight;		// Right			[Normalized]
		core::math::vec<float, 3>													DirectionUp;		// Up				[Normalized]
		core::math::vec<float, 3>													DirectionFront;		// Backward			[Normalized]
		core::math::vec<float, 3>													LinearMomentum;		// Linear Momentum	[kg*m/s]
		core::math::vec<float, 3>													AngularMomentum;	// Angular Momentum [kg*m/s]

		// * Object Modes:
		// * This section controls the broader
		motion 																		Motion;				// Informs how the phys engine will treat the object.
		bool 																		Gravity;			// Determines if the object is affected by gravity.
		bool 																		Collision;			// Determines if object is affected by collisions.

		std::vector<std::shared_ptr<core::io::file>> 								Asset;

		// ! ----- Device Data ----- ! //
		// ^ This is the data that exists on the GPU.

		std::shared_ptr<core::gcl::context> 										Context;
		std::shared_ptr<core::phys::mesh>											CollisionBox;
		std::shared_ptr<core::gfx::model>											Model;
		std::shared_ptr<core::gcl::buffer> 											UniformBuffer;
		std::map<subject*, std::vector<std::vector<core::gfx::draw_call>>>			Renderer;

		object(std::shared_ptr<core::gcl::context> aContext, stage* aStage, std::string aName, core::math::vec<float, 3> aPosition = { 0.0f, 0.0f, 0.0f }, core::math::vec<float, 2> aDirection = { 0.0f, 0.0f });
		~object();

		virtual bool is_subject();

		virtual void input(const core::hid::input& aInput);
		virtual void update(double aDeltaTime, core::math::vec<float, 3> aAppliedForce = { 0.0f, 0.0f, 0.0f }, core::math::vec<float, 3> aAppliedTorque = { 0.0f, 0.0f, 0.0f });
		virtual std::vector<core::gfx::draw_call> draw(subject* aSubject);

	protected:

		core::math::vec<float, 3> InputVelocity;
		core::math::vec<float, 3> InputForce;

	};
	
}

#endif // GEODESY_CORE_OBJECT_H
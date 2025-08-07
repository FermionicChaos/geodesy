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

	// Compile-time string length calculation
	constexpr size_t compile_time_strlen(const char* str) {
		size_t len = 0;
		while (str[len] != '\0') {
			++len;
		}
		return len;
	}

	// FNV-1a hash function for compile-time type ID generation
	constexpr uint32_t fnv1a_hash(const char* str, size_t len) {
		uint32_t hash = 2166136261u;  // FNV offset basis
		for (size_t i = 0; i < len; ++i) {
			hash ^= static_cast<uint32_t>(str[i]);
			hash *= 16777619u;  // FNV prime
		}
		return hash;
	}

	// Generate unique RTTI ID from type name at compile time
	template<typename T>
	constexpr uint32_t generate_rttiid() {
		// Use compiler-specific type name extraction
		#if defined(__GNUC__) || defined(__clang__)
			constexpr const char* func_name = __PRETTY_FUNCTION__;
		#elif defined(_MSC_VER)
			constexpr const char* func_name = __FUNCSIG__;
		#else
			constexpr const char* func_name = __func__;
		#endif
		
		constexpr size_t len = compile_time_strlen(func_name);
		return fnv1a_hash(func_name, len);
	}
	
	class object : public core::gfx::node {
	public:

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
			subject* 												Subject;
			object* 												Object;
			std::vector<std::vector<std::shared_ptr<draw_call>>> 	DrawCallList;

			renderer();
			renderer(subject* aSubject, object* aObject);
			virtual ~renderer();

			std::vector<std::shared_ptr<draw_call>> operator[](size_t aIndex) const;

			virtual void update(
				double aDeltaTime = 0.0f, 
				double aTime = 0.0f
			);

		};

		struct creator {
			std::string 					Name;
			uint32_t						RTTIID;
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

		// Runtime Type Information (RTTI) ID for the object.
		constexpr static uint32_t rttiid = generate_rttiid<object>();

		// ! ----- Host Data ----- ! //
		// ^ This data exists in Host memory.

		// * Object Metadata
		stage*																		Stage;
		engine*																		Engine;
		std::mutex																	Mutex;

		// * Object Input and Physics
		std::string																	Name;				// Name of the object.
		uint32_t																	RTTIID;
		float 																		Theta, Phi;			// Radians			[rad]
		std::vector<float> 															AnimationWeights;
		std::vector<std::shared_ptr<core::io::file>> 								Asset;

		// ! ----- Device Data ----- ! //
		// ^ This is the data that exists on the GPU.
		std::shared_ptr<core::gfx::model>											Model;
		std::vector<core::phys::node*> 												LinearizedNodeTree;
		std::vector<core::gfx::mesh::instance*> 									TotalMeshInstance;
		std::map<subject*, std::shared_ptr<renderer>>								Renderer;

		object(std::shared_ptr<core::gpu::context> aContext, stage* aStage, creator* aCreator);
		~object();

		void copy_data(const core::phys::node* aNode) override;

		virtual bool is_subject();
		virtual void input(const core::hid::input& aInput);
		virtual void host_update(
			double 										aDeltaTime = 0.0f, 
			double 										aTime = 0.0f, 
			const std::vector<core::phys::force>& 		aAppliedForces = {}
		) override;
		virtual void device_update(
			double 										aDeltaTime = 0.0f, 
			double 										aTime = 0.0f, 
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
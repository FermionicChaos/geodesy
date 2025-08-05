#pragma once
#ifndef GEODESY_BLTN_OBJ_CAMERAVR_H
#define GEODESY_BLTN_OBJ_CAMERAVR_H

#include <geodesy/engine.h>

// Forward declarations for OpenXR handle types
struct XrInstance_T;
struct XrSession_T;
struct XrSpace_T;

// OpenXR handle typedefs (opaque pointers)
typedef struct XrInstance_T* XrInstance;
typedef struct XrSession_T* XrSession;
typedef struct XrSpace_T* XrSpace;
typedef uint64_t XrSystemId;

// OpenXR null handle constants
#define XR_NULL_HANDLE nullptr
#define XR_NULL_SYSTEM_ID 0

namespace geodesy::bltn::obj {

	class cameravr : public runtime::subject {
	public:

		struct creator : public runtime::subject::creator {
			/*
			Image Usage
			Image Format
			Color Space
			*/
			creator();
		};

		// Runtime Type Information (RTTI) ID for the cameravr class.
		constexpr static uint32_t rttiid = geodesy::runtime::generate_rttiid<cameravr>();
		static XrInstance 				Instance; 				// OpenXR Instance handle
		static std::set<std::string> 	EngineExtensionsModule;
		static std::set<std::string> 	EngineLayersModule;
		static std::set<std::string> 	ContextExtensionsModule;
		static std::set<std::string> 	ContextLayersModule;
		static bool initialize(std::set<std::string> aLayerList = {}, std::set<std::string> aExtensionList = {});
		static void terminate();

		XrSystemId 						SystemId; 				// HMD
		XrSession 						Session; 				// XR Application Session



		class geometry_buffer : public framechain {

		};

		cameravr(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aCreator);

	};

}

#endif // !GEODESY_BLTN_OBJ_CAMERAVR_H
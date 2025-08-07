#pragma once
#ifndef GEODESY_BLTN_OBJ_CAMERAVR_H
#define GEODESY_BLTN_OBJ_CAMERAVR_H

#include <geodesy/engine.h>

// Forward declarations for OpenXR handle types
struct XrInstance_T;
struct XrSession_T;
struct XrSpace_T;
struct XrViewConfigurationView;

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

		enum form_factor {
			HEAD_MOUNTED_DISPLAY = 1,
			HANDHELD_DISPLAY = 2,
			OTHER = 3
		};

		class swapchain : public framechain {
		public:

			struct creator {
				// Empty for now
				creator() {}
			};


			swapchain(std::shared_ptr<core::gpu::context> aContext, XrSession aSession, const creator& aCreator);

		};

		struct creator : public runtime::subject::creator {
			creator();
		};

		// Runtime Type Information (RTTI) ID for the cameravr class.
		constexpr static uint32_t rttiid = geodesy::runtime::generate_rttiid<cameravr>();
		static XrInstance 								Instance; 				// OpenXR Instance handle
		static XrSystemId 								SystemID; 				// HMD
		static std::vector<XrViewConfigurationView> 	Views;
		static std::set<std::string> 					EngineExtensionsModule;
		static std::set<std::string> 					EngineLayersModule;
		static std::set<std::string> 					ContextExtensionsModule;
		static std::set<std::string> 					ContextLayersModule;
		static bool initialize(form_factor aFormFactor, std::set<std::string> aLayerList = {}, std::set<std::string> aExtensionList = {});
		static void terminate();

		XrSession 										Session; 				// XR Application Session

		cameravr(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aCreator);

	};

}

#endif // !GEODESY_BLTN_OBJ_CAMERAVR_H
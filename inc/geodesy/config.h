#pragma once
#ifndef GEODESY_CONFIG_H
#define GEODESY_CONFIG_H

// --------------- C Standard Library Includes --------------- //

// --------------- C++ Standard Library Includes --------------- //
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <set>
#include <map>
#include <list>
#include <mutex>
#include <thread>

#define GEODESY_ENGINE_VERSION_MAJOR 0
#define GEODESY_ENGINE_VERSION_MINOR 1
#define GEODESY_ENGINE_VERSION_PATCH 5

// #if ((defined(_WIN32) || defined(_WIN64)) && !defined(NDEBUG))
// #pragma comment(linker, "/SUBSYSTEM:WINDOWS")
// #define main wWinMain
// #endif

// #pragma comment( linker, "/subsystem:"windows" /entry:"mainCRTStartup"" ) 

// Forward Declarations of Core Objects.
namespace geodesy {

	// Generates easily debuggable shared pointers.
	template <typename T, typename... Args>
	std::shared_ptr<T> make(Args&&... args) {
    	return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
	}

	// Namespace forward declarations.
	namespace core {
		namespace io {};
		namespace math {};
		namespace util {};
		namespace lgc {};
		namespace phys {};
		namespace hid {};
		namespace gpu {
			class device;
			class command_pool;
			class semaphore_pool;
			class command_batch;
			class submission_batch;
			class buffer;
			class image;
			class shader;
			class descriptor;
			class framebuffer;
			class pipeline;
			class framechain;
			class acceleration_structure;
			class context;
		}
		namespace gfx {
			class mesh;
			class node;
			class model;
		}
		namespace sfx {};
	}
	namespace runtime {
		class object;
		class subject;
		class stage;
	}
	class engine;
	namespace bltn {
		namespace obj {
			class system_display;
			class system_window;
		}
	}
}

#endif // !GEODESY_CONFIG_H

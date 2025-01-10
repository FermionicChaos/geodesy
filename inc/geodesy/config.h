#pragma once
#ifndef GEODESY_CONFIG_H
#define GEODESY_CONFIG_H

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <set>
#include <map>

#include <mutex>
#include <thread>
#include <memory>

#define GEODESY_ENGINE_VERSION_MAJOR 0
#define GEODESY_ENGINE_VERSION_MINOR 1
#define GEODESY_ENGINE_VERSION_PATCH 5

// Disables Terminal Window, for windows only
#if defined(_WIN32) || defined(_WIN64)
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

// Forward Declarations of Core Objects.
namespace geodesy {

	// Quality of life function.
	template <typename T, typename... Args>
	std::shared_ptr<T> make(Args&&... args) {
    	return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
	}

	// Recursive case for higher dimensions
	template <typename T, typename... Sizes>
	auto make_multi_vector(std::size_t size, Sizes... sizes) {
	    return std::vector<decltype(make_multi_vector<T>(sizes...))>(
	        size, make_multi_vector<T>(sizes...)
	    );
	}

	class engine;
	namespace core {
		namespace gcl {
			class context;
		}
		namespace gfx {
			class model;
		}
	}
	namespace ecs {
		class object;
		class subject;
		class stage;
	}
	namespace bltn {
		namespace obj {
			class system_display;
			class system_window;
		}
	}
}

#endif // !GEODESY_CONFIG_H

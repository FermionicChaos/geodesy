#include <iostream>

// Includes core engine, and base types.
#include <geodesy/engine.h>

// Includes engine built in object primitives.
#include <geodesy/bltn.h>

using namespace geodesy;
using namespace bltn;
using namespace obj;

// #if ((defined(_WIN32) || defined(_WIN64)) && !defined(NDEBUG))
// #pragma comment(linker, "/SUBSYSTEM:WINDOWS")
// #define main wWinMain
// #endif

// #pragma comment( linker, "/subsystem:"windows" /entry:"mainCRTStartup"" ) 

// Using entry point for app.
int main(int aCmdArgCount, char* aCmdArgList[]) {

	// Initialize all third party libraries.
	if (!geodesy::engine::initialize()) return -1;

	std::vector<const char*> CommandLineArguments(aCmdArgCount);
	for (int i = 0; i < aCmdArgCount; i++) {
		CommandLineArguments[i] = aCmdArgList[i];
	}

	for (size_t i = 0; i < CommandLineArguments.size(); i++) {
		std::cout << "CommandLineArg[" << i << "] = " << CommandLineArguments[i] << std::endl;
	}

	// Load select layers desired for engine.
	std::vector<const char*> LayerList = {
		"VK_LAYER_KHRONOS_validation"
	};

	// Load selected extensions so engine can render to system windows.
	std::vector<const char*> ExtensionList = system_window::engine_extensions();

	try {

		// Initialize Engine.
		geodesy::engine Engine(CommandLineArguments, LayerList, ExtensionList);

		// Initialize User App
		bltn::unit_test UnitTest(&Engine);

		// Run User App
		Engine.run(&UnitTest);
		
	} catch (geodesy::core::util::log EngineLog) {
		//std::cout << EngineLog << std::endl;
		return -1;
	}

	// Terminate all third party libraries.
	geodesy::engine::terminate();

	return 0;
}

#include <iostream>

// Includes core engine, and base types.
#include <geodesy/engine.h>

// Includes engine built in object primitives.
#include <geodesy/bltn.h>

// #if ((defined(_WIN32) || defined(_WIN64)) && !defined(NDEBUG))
// #pragma comment(linker, "/SUBSYSTEM:WINDOWS")
// #define main wWinMain
// #endif

// #pragma comment( linker, "/subsystem:"windows" /entry:"mainCRTStartup"" ) 

// Using entry point for app.
int main(int aCmdArgCount, char* aCmdArgList[]) {

	std::vector<const char*> CommandLineArguments(aCmdArgCount);
	for (int i = 0; i < aCmdArgCount; i++) {
		CommandLineArguments[i] = aCmdArgList[i];
	}

	for (size_t i = 0; i < CommandLineArguments.size(); i++) {
		std::cout << "CommandLineArg[" << i << "] = " << CommandLineArguments[i] << std::endl;
	}

	try {

		// Initialize Engine.
		geodesy::engine Engine(CommandLineArguments, {}, {});

		// Initialize User App
		geodesy::bltn::unit_test UnitTest(&Engine);

		// Run User App
		Engine.run(&UnitTest);
		
	} catch (geodesy::core::util::log EngineLog) {
		//std::cout << EngineLog << std::endl;
		return -1;
	}
	return 0;
}

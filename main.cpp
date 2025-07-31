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

	// ===== Load Engine Layers ===== //
	std::set<std::string> LayerList = {};
	// Insert engine layers into the layer list.
	LayerList.insert(engine::EngineLayersModule.begin(), engine::EngineLayersModule.end());
	// Insert System window layers into the layer list.
	LayerList.insert(system_window::EngineLayersModule.begin(), system_window::EngineLayersModule.end());

	// ===== Load Engine Extensions ===== //
	std::set<std::string> ExtensionList = {};
	// Insert engine extensions into the extension list.
	ExtensionList.insert(engine::EngineExtensionsModule.begin(), engine::EngineExtensionsModule.end());
	// Insert System window extensions into the extension list.
	ExtensionList.insert(system_window::EngineExtensionsModule.begin(), system_window::EngineExtensionsModule.end());

	try {
		geodesy::engine Engine(CommandLineArguments, LayerList, ExtensionList);
		{
			// Initialize User App
			bltn::unit_test UnitTest(&Engine);

			// Run User App
			Engine.run(&UnitTest);
		}
	}
	catch (const geodesy::core::util::log& Logger) {
		std::cout << "log" << std::endl;
		for (size_t i = 0; i < Logger.Message.size(); i++) {
			const auto& Message = Logger.Message[i];
			std::cerr << "Log[" << i << "]:"
			<< " | Type - " << geodesy::core::util::log::message::type_to_string(Message.Type)
			<< " | Reporter - " << geodesy::core::util::log::message::api_to_string(Message.IssuerAPI)
			<< " | Code - " << geodesy::core::util::log::message::code_to_string(Message.Code)
			<< " | Message - " << Message.Content << " |" << std::endl;
		}
		return -1;

	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	}

	// Terminate all third party libraries.
	geodesy::engine::terminate();

	return 0;
}

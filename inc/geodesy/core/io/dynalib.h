#pragma once
#ifndef GEODESY_CORE_IO_DYNALIB_H
#define GEODESY_CORE_IO_DYNALIB_H

/*
* This will be for loading in runtime libraries that
* utilize the engine. The idea is to allow for loadable
* run time mods through dynamically loaded libraries.
* Most likely to extend object_t
*/

#include "../../config.h"
#include "file.h"

namespace geodesy::core::io {

	class dynalib : public file {
	public:
		
		void* Handle;

		dynalib();
		dynalib(std::string aLibraryPath);
		~dynalib();

		void* get_function_pointer(std::string aFunctionDeclaration);

	};

}

#endif // !GEODESY_CORE_IO_DYNALIB_H

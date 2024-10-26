#include <geodesy/core/io/dynalib.h>

#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__APPLE__) || defined(MACOSX)
#include <dlfcn.h>
#elif defined(__linux__) && !defined(__ANDROID__)
#include <dlfcn.h>
#elif defined(__ANDROID__)
#include <dlfcn.h>
#endif

namespace geodesy::core::io {

	dynalib::dynalib() {
		this->Handle = NULL;
	}

	dynalib::dynalib(std::string aFilePath) : dynalib() {
		if (aFilePath.length() == 0) return;
#if defined(_WIN32) || defined(_WIN64)
		wchar_t* WideCharPath = (wchar_t*)malloc((aFilePath.length() + 1) * sizeof(wchar_t));
		if (WideCharPath != NULL) {
			WideCharPath[aFilePath.length()] = '\0';
			mbstowcs(WideCharPath, aFilePath.c_str(), aFilePath.length()); // maybe include size, but just char count
			this->Handle = reinterpret_cast<void*>(LoadLibraryW(WideCharPath));
			free(WideCharPath);
		}
#elif defined(__APPLE__) || defined(MACOSX)

#elif defined(__linux__) && !defined(__ANDROID__)
		this->Handle = dlopen(aFilePath.c_str(), RTLD_LAZY);
#endif
	}

	dynalib::~dynalib() {
#if defined(_WIN32) || defined(_WIN64)
		FreeLibrary(reinterpret_cast<HMODULE>(this->Handle));
#elif defined(__APPLE__) || defined(MACOSX)

#elif defined(__linux__) && !defined(__ANDROID__)
		dlclose(this->Handle);
#endif
	}

	// Fetch function pointer from loaded library.
	void* dynalib::get_function_pointer(std::string aFunctionDeclaration) {
		void* FunctionPointer = NULL;
#if defined(_WIN32) || defined(_WIN64)
		FunctionPointer = reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(this->Handle), LPCSTR(aFunctionDeclaration.c_str())));
#elif defined(__APPLE__) || defined(MACOSX)

#elif defined(__linux__) && !defined(__ANDROID__)
		FunctionPointer = dlsym(this->Handle, aFunctionDeclaration.c_str());
#elif defined(__ANDROID__)

#endif
		return FunctionPointer;
	}

}

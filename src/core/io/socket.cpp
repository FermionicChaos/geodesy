#include <geodesy/core/io/socket.h>

/* --------------- Platform Dependent Libraries --------------- */
// Platform-specific includes and definitions
#if defined(_WIN32) || defined(_WIN64)
	// Windows-specific includes
	#include <winsock2.h>
	#include <ws2tcpip.h>
	using native_socket_type = SOCKET;
	const native_socket_type INVALID_SOCKET_VALUE = INVALID_SOCKET;
	// #pragma comment(lib, "ws2_32.lib")
	static WSADATA WsaData;
#elif defined(__linux__) || defined(__APPLE__) || defined(MACOSX)
	// Linux and macOS-specific includes
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	using native_socket_type = int;
    const native_socket_type INVALID_SOCKET_VALUE = -1;
#endif

// include runtime error
#include <stdexcept>

namespace geodesy::core::io {

	int socket_t::initialize() {
#if defined(_WIN32) || defined(_WIN64)
		return WSAStartup(MAKEWORD(2, 2), &WsaData);
#elif defined(__linux__) || defined(__APPLE__) || defined(MACOSX)
		return 0;
#endif
	}

	int socket_t::terminate() {
#if defined(_WIN32) || defined(_WIN64)
		return WSACleanup();
#elif defined(__linux__) || defined(__APPLE__) || defined(MACOSX)
		return 0;
#endif
	}

} // namespace geodesy::core::io

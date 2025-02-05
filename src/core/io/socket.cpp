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
    using SocketType = int;
    const SocketType INVALID_SOCKET_VALUE = -1;
#endif

namespace geodesy::core::io {

	bool socket_t::initialize() {
#if defined(_WIN32) || defined(_WIN64)
		return (WSAStartup(MAKEWORD(2, 2), &WsaData) == 0);
#elif defined(__linux__) || defined(__APPLE__) || defined(MACOSX)
		return true;
#endif
	}

	void socket_t::terminate() {
#if defined(_WIN32) || defined(_WIN64)
		WSACleanup();
#endif	
	}

	socket_t::socket_t(std::string aIpAddress, unsigned short aPortNumber, address_family aAddressFamily, protocol aProtocol) : socket_t() {
		this->IpAddress = aIpAddress;
		this->PortNumber = aPortNumber;
		this->AddressFamily = aAddressFamily;
		this->Protocol = aProtocol;

		int Family = (this->AddressFamily == IPV4) ? AF_INET : AF_INET6;
		int Type = (this->Protocol == TCP) ? SOCK_STREAM : SOCK_DGRAM;
		int Protocol = (this->Protocol == TCP) ? IPPROTO_TCP : IPPROTO_UDP;

		// Allocate Memory for Socket Handle
		this->NativeHandle = malloc(sizeof(native_socket_type));
		if (this->NativeHandle == NULL) {
			std::runtime_error("Error: Failed to allocate memory for socket handle.");
		}

		// Create Socket Handle
		*(native_socket_type*)this->NativeHandle = socket(Family, Type, Protocol);
#if defined(_WIN32) || defined(_WIN64)
		if (*(native_socket_type*)this->NativeHandle == INVALID_SOCKET_VALUE) {
#elif defined(__linux__) || defined(__APPLE__) || defined(MACOSX)
		if (*(native_socket_type*)this->NativeHandle == INVALID_SOCKET_VALUE) {
#endif
			std::runtime_error("Error: Failed to create socket handle.");
		}

		// Determine if Server or Client
		if (this->IpAddress == "") {
			// This is intended as a server.
			switch(this->AddressFamily) {
			default:
				break;
			case IPV4: {
					// Create Address Structure
					sockaddr_in Address;
					Address.sin_family = AF_INET;
					Address.sin_port = htons(this->PortNumber);
					Address.sin_addr.s_addr = INADDR_ANY;
					// Bind Address to Socket
					if (bind(*(native_socket_type*)this->NativeHandle, (sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR) {
						std::runtime_error("Error: Failed to bind address to socket.");
					}
				}
				break;
			case IPV6: {
					// Create Address Structure
					sockaddr_in6 Address;
					Address.sin6_family = AF_INET6;
					Address.sin6_port = htons(this->PortNumber);
					Address.sin6_addr = in6addr_any;
					// Bind Address to Socket
					if (bind(*(native_socket_type*)this->NativeHandle, (sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR) {
						std::runtime_error("Error: Failed to bind address to socket.");
					}
				}
				break;
			}

			// Listen for incoming connections.

		}
		else {
			// This is intended as a client.
			switch(this->AddressFamily) {
			default:
				break;
			case IPV4: {
					// Create Address Structure
					sockaddr_in Address;
					Address.sin_family = AF_INET;
					Address.sin_port = htons(this->PortNumber);
					inet_pton(AF_INET, this->IpAddress.c_str(), &Address.sin_addr);
					// Connect to Address
					if (connect(*(native_socket_type*)this->NativeHandle, (sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR) {
						std::runtime_error("Error: Failed to connect to address.");
					}
				}
				break;
			case IPV6: {
					// Create Address Structure
					sockaddr_in6 Address;
					Address.sin6_family = AF_INET6;
					Address.sin6_port = htons(this->PortNumber);
					inet_pton(AF_INET6, this->IpAddress.c_str(), &Address.sin6_addr);
					// Connect to Address
					if (connect(*(native_socket_type*)this->NativeHandle, (sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR) {
						std::runtime_error("Error: Failed to connect to address.");
					}
				}
				break;
			}
		}

	}

	socket_t::~socket_t() {
#if defined(_WIN32) || defined(_WIN64)
		closesocket(*(SOCKET*)this->NativeHandle);
#elif defined(__linux__) || defined(__APPLE__) || defined(MACOSX)
		close(*(int*)this->NativeHandle);
#endif
	}

	socket_t::socket_t() {
		this->NativeHandle = NULL;
		this->IpAddress = "";
		this->PortNumber = 0;
		this->AddressFamily = IPV4;
		this->Protocol = UDP;
	}

} // namespace geodesy::core::io

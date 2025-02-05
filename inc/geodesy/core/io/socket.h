#pragma once
#ifndef GEODESY_CORE_IO_SOCKET_H
#define GEODESY_CORE_IO_SOCKET_H

#include "../../config.h"

namespace geodesy::core::io {

	class socket_t {
	public:

		enum address_family {
			IPV4,
			IPV6
		};

		enum protocol {
			TCP,
			UDP
		};

		static bool initialize();
		static void terminate();

		socket_t(std::string aIpAddress, unsigned short aPortNumber, address_family aAddressFamily = IPV4, protocol aProtocol = UDP);
		~socket_t();

	private:

		void* NativeHandle;
		std::string IpAddress;
		unsigned short PortNumber;
		address_family AddressFamily;
		protocol Protocol;

		socket_t();

	};

}

#endif // !GEODESY_CORE_IO_SOCKET_H
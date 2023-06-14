#pragma once
#include <string>
#include "../nettype.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

class InetAddress
{
public:
	static bool resolve(std::string hostname, InetAddress* result);

public:
	explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
	InetAddress(std::string ip, uint16_t port, bool ipv6 = false);

	uint16_t port();
	std::string toIp();
	std::string toIpPort();
	sa_family_t family();

	const struct sockaddr* getSockAddr();
	const struct sockaddr* getSockAddr4();
	const struct sockaddr* getSockAddr6();

	uint32_t ipv4NetEndian() const;
	uint16_t portNetEndian() const;

private:
	union
	{
		struct sockaddr_in m_addr;
		struct sockaddr_in6 m_addr6;
	};
};
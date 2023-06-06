#pragma once
#include <string>
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

	std::string toIp();
	std::string toIpPort();
	uint16_t port();

private:
	union
	{
		struct sockaddr_in addr_;
		struct sockaddr_in6 addr6_;
	};
};
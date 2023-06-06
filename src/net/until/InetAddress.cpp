#include "InetAddress.h"
#include "common/until/encoding.h"
#include "common/log/log.h"
#include <sstream>
#include <iostream>

bool InetAddress::resolve(std::string hostname, InetAddress* result)
{
#ifdef _WIN32
	struct addrinfo hints, * res;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	WSADATA wsaData;
	int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (r != 0)
	{
		LOG_ERROR << "WSAStartup failed with error: " << result;
		return false;
	}
	int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
	WSACleanup();
	if (status != 0) {
		// 解析失败
		LOG_ERROR << "getaddrinfo failed with error: " << status;
		return false;
	}

	if (res->ai_family == AF_INET)
	{
		struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
		result->addr_.sin_family = AF_INET;
		result->addr_.sin_port = ipv4->sin_port;
		LOG_DEBUG << "AF_INET port: " << ipv4->sin_port;
		result->addr_.sin_addr = ipv4->sin_addr;
	}
	else if (res->ai_family == AF_INET6)
	{
		struct sockaddr_in6* ipv6 = reinterpret_cast<struct sockaddr_in6*>(res->ai_addr);
		result->addr6_.sin6_family = AF_INET6;
		result->addr6_.sin6_port = ipv6->sin6_port;
		result->addr6_.sin6_addr = ipv6->sin6_addr;
	}
	else {
		// 不支持的地址族
		freeaddrinfo(res);
		return false;
	}
	freeaddrinfo(res);
	return true;

#else
	struct hostent hostbuf, * hp;
	char buffer[1024];
	std::memset(&hostbuf, 0, sizeof(hostbuf));
	std::memset(buffer, 0, sizeof(buffer));
	int status = gethostbyname_r(hostname.c_str(), &hostbuf, buffer, sizeof(buffer), &hp, &err);

	if (status != 0 || hp == nullptr)
	{
		LOG_DEBUG << "Failed to resolve hostname: " << hostname;
		return false;
	}

	char ipStr[INET_ADDRSTRLEN];
	const char* result = inet_ntop(hp->h_addrtype, hp->h_addr_list[0], ipStr, sizeof(ipStr));
	if (result == nullptr)
	{
		LOG_DEBUG << "Failed to convert IP address to string.";
		return false;
	}
	ip = ipStr;
	return true;
#endif
}

InetAddress::InetAddress(uint16_t port /*= 0*/, bool loopbackOnly /*= false*/, bool ipv6 /*= false*/)
{
	std::memset(&addr_, 0, sizeof(addr_));

	if (ipv6)
	{
		addr6_.sin6_family = AF_INET6;
		addr6_.sin6_port = (uint16_t)HostToBigEndian((int16_t)port);
		if (loopbackOnly) 
		{
			inet_pton(AF_INET6, "::1", &addr6_.sin6_addr);
		}
		else 
		{
			addr6_.sin6_addr = in6addr_any;
		}
	}
	else 
	{
		addr_.sin_family = AF_INET;
		addr_.sin_port = (uint16_t)HostToBigEndian((int16_t)port);
		if (loopbackOnly) 
		{
			inet_pton(AF_INET, "127.0.0.1", &addr_.sin_addr);
		}
		else 
		{
			addr_.sin_addr.s_addr = INADDR_ANY;
		}
	}
}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6 /*= false*/)
{
	std::memset(&addr_, 0, sizeof(addr_));
	if (ipv6 || strchr(ip.c_str(), ':'))
	{
		addr6_.sin6_family = AF_INET6;
		addr6_.sin6_port = (uint16_t)HostToBigEndian((int16_t)port);
		inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr);
	}
	else
	{
		addr_.sin_family = AF_INET;
		addr_.sin_port = (uint16_t)HostToBigEndian((int16_t)port);
		inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
	}
}

std::string InetAddress::toIp()
{
	char buffer[INET6_ADDRSTRLEN];
	if (addr_.sin_family == AF_INET)
	{
		// IPv4 地址
		const char* ip = inet_ntop(AF_INET, &(addr_.sin_addr), buffer, INET_ADDRSTRLEN);
		if (ip != nullptr)
		{
			return std::string(ip);
		}
	}
	else if (addr_.sin_family == AF_INET6)
	{
		// IPv6 地址
		const char* ip = inet_ntop(AF_INET6, &(addr6_.sin6_addr), buffer, INET6_ADDRSTRLEN);
		if (ip != nullptr)
		{
			return std::string(ip);
		}
	}
	return "";
}

std::string InetAddress::toIpPort()
{
	std::stringstream ss;
	if (addr_.sin_family == AF_INET6)
	{
		ss << "[";
	}
	ss << toIp();
	if (addr_.sin_family == AF_INET6)
	{
		ss << "]";
	}
	ss << ":";
	ss << (uint16_t)BigEndianToHost((int16_t)addr_.sin_port);
	return ss.str();
}

uint16_t InetAddress::port()
{
	return BigEndianToHost((int16_t)addr_.sin_port);
}


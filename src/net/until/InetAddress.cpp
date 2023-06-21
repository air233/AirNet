#include "InetAddress.h"
#include "../../common/until/encoding.h"
#include "../../common/log/log.h"
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
		result->m_addr.sin_family = AF_INET;
		result->m_addr.sin_port = ipv4->sin_port;
		result->m_addr.sin_addr = ipv4->sin_addr;
	}
	else if (res->ai_family == AF_INET6)
	{
		struct sockaddr_in6* ipv6 = reinterpret_cast<struct sockaddr_in6*>(res->ai_addr);
		result->m_addr6.sin6_family = AF_INET6;
		result->m_addr6.sin6_port = ipv6->sin6_port;
		result->m_addr6.sin6_addr = ipv6->sin6_addr;
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
	std::memset(&m_addr, 0, sizeof(m_addr));

	if (ipv6)
	{
		m_addr6.sin6_family = AF_INET6;
		m_addr6.sin6_port = HostToBigEndian(port);
		if (loopbackOnly) 
		{
			inet_pton(AF_INET6, "::1", &m_addr6.sin6_addr);
		}
		else 
		{
			m_addr6.sin6_addr = in6addr_any;
		}
	}
	else 
	{
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = HostToBigEndian(port);
		if (loopbackOnly) 
		{
			inet_pton(AF_INET, "127.0.0.1", &m_addr.sin_addr);
		}
		else 
		{
			m_addr.sin_addr.s_addr = INADDR_ANY;
		}
	}
}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6 /*= false*/)
{
	std::memset(&m_addr, 0, sizeof(m_addr));
	if (ipv6 || strchr(ip.c_str(), ':'))
	{
		m_addr6.sin6_family = AF_INET6;
		m_addr6.sin6_port = HostToBigEndian(port);
		inet_pton(AF_INET6, ip.c_str(), &m_addr6.sin6_addr);
	}
	else
	{
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = HostToBigEndian(port);
		inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr);
	}
}

InetAddress::InetAddress(const struct sockaddr_in& addr):m_addr(addr)
{
}

InetAddress::InetAddress(const struct sockaddr_in6& addr):m_addr6(addr)
{
}

InetAddress::InetAddress(const struct sockaddr_storage addr)
{
	if (addr.ss_family == AF_INET)
	{
		m_addr = *((sockaddr_in*)&addr);
	}
	else
	{
		m_addr6 = *((sockaddr_in6*)&addr);
	}
}

std::string InetAddress::toIp()
{
	char buffer[INET6_ADDRSTRLEN];
	if (m_addr.sin_family == AF_INET)
	{
		// IPv4 地址
		const char* ip = inet_ntop(AF_INET, &(m_addr.sin_addr), buffer, INET_ADDRSTRLEN);
		if (ip != nullptr)
		{
			return std::string(ip);
		}
	}
	else if (m_addr.sin_family == AF_INET6)
	{
		// IPv6 地址
		const char* ip = inet_ntop(AF_INET6, &(m_addr6.sin6_addr), buffer, INET6_ADDRSTRLEN);
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
	if (m_addr.sin_family == AF_INET6)
	{
		ss << "[";
	}
	ss << toIp();
	if (m_addr.sin_family == AF_INET6)
	{
		ss << "]";
	}
	ss << ":";
	ss << BigEndianToHost(m_addr.sin_port);
	return ss.str();
}

uint16_t InetAddress::port()
{
	return BigEndianToHost(m_addr.sin_port);
}

sa_family_t InetAddress::family() const
{
	return m_addr.sin_family;
}

struct sockaddr* InetAddress::getSockAddr()
{
	if (family() == AF_INET)
	{
		return getSockAddr4();
	}
	else
	{
		return getSockAddr6();
	}
}

socklen_t InetAddress::getSockAddrLen()
{
	if (family() == AF_INET)
	{
		return sizeof sockaddr_in;
	}
	else
	{
		return sizeof sockaddr_in6;
	}
}

struct sockaddr* InetAddress::getSockAddr4()
{
	return static_cast<struct sockaddr*>((void*)(&m_addr));
}

struct sockaddr* InetAddress::getSockAddr6()
{
	return static_cast<struct sockaddr*>((void*)(&m_addr6));
}

uint32_t InetAddress::ipv4NetEndian() const
{
	return m_addr.sin_addr.s_addr;
}

uint16_t InetAddress::portNetEndian() const
{
	return m_addr.sin_port;
}


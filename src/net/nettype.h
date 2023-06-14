#pragma once
#include <cstdint>

#ifdef _WIN32
#define sa_family_t ADDRESS_FAMILY
#else
#define SOCKET int
#define INVALID_SOCKET -1
#endif

enum NetMode
{
	NONE,
	TCP,
	UDP,
	KCP,
	MAX,
};

enum NetStatus
{
	Disconnected,
	Connecting,
	Connected,
	Disconnecting
};

enum
{
	BuffMax = 10240,
};
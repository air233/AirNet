#pragma once
#include <cstdint>

#ifdef _WIN32
#define sa_family_t ADDRESS_FAMILY
#define ssize_t	SSIZE_T 
#else
#define SOCKET int
#define INVALID_SOCKET -1
#endif

#define INVALID_NET_ID 0

typedef struct TCPConfig
{
	bool delay;
	bool reuse;
	bool keepalive;
	bool lingerzero;
}TCPServerConfig;

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
	Listening,
	Connecting,
	Connected,
	Disconnecting
};

enum
{
	BuffMax = 5 * 1024 * 1024,/*5Mib*/
};
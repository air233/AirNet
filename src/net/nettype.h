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

enum JobType
{
	JobNone,
	JobAccept,
	JobConnect,
	JobDisconnect,
	JobReveive,
	JobError,
};

enum NetErrCode {
	NET_TIMEOUT = -7,
	NET_CONNECT_FAIL = -6,
	NET_SYSTEM_ERROR = -5,
	NET_RECV_ERROR = -4,
	NET_SEND_ERROR = -3,
	NET_SEND_OVERFLOW = -2,
	NET_PACKET_ERROR = -1,
	NET_SUCCESS = 0
};

enum
{
	BuffMax = 5 * 1024 * 1024,/*5Mib*/
};


struct ConnectInfo
{
	uint64_t m_net_id;
	uint64_t m_timeout;
};

struct ConnectInfoComparator
{
	bool operator()(const ConnectInfo& lhs, const ConnectInfo& rhs) const
	{
		return lhs.m_net_id < rhs.m_net_id;
	}
};
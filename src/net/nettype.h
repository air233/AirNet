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

/*��д�¼�*/
#define IONONE 0
#define IOREAD 1
#define IOWRIT 2
#define IOREADWRIT 3


typedef struct TCPConfig
{
	bool delay;
	bool reuse;
	bool keepalive;
	bool lingerzero;
}TCPServerConfig;

enum class NetMode : int
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
	JobNewConn,
	JobConnect,
	JobDisconnect,
	JobReveive,
	JobReveiveFrom,
	JobError,
};

enum NetErrCode {
	NET_BUFFER_TOO_LARGE = -9,
	NET_OBJ_ERR = -8,
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
	BuffMax = 10 * 1024 * 1024,/*5Mib*/
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


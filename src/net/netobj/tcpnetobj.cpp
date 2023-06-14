#include "tcpnetobj.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

TcpNetObj::TcpNetObj(uint64_t net_id, SOCKET fd):
	BaseNetObj(net_id,fd)
{
	m_net_mode = TCP;
}

TcpNetObj::~TcpNetObj()
{

}

void TcpNetObj::setNoDelay(bool on)
{
	int optionValue = on ? 1 : 0;
	::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&optionValue, sizeof(optionValue));
}

void TcpNetObj::setReuseAddr(bool on)
{
	int optionValue = on ? 1 : 0;
	::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optionValue, sizeof(optionValue));
}

void TcpNetObj::setKeepAlive(bool on)
{
	int optionValue = on ? 1 : 0;
	::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&optionValue, sizeof(optionValue));
}



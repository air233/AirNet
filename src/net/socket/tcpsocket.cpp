#include "tcpsocket.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

TCPSocket::TCPSocket()
{
	m_fd = -1;
	m_socket_type = SOCKET_TYPE.TCP;
}

TCPSocket::~TCPSocket()
{

}

void TCPSocket::setNoDelay(bool on)
{
	int optionValue = on ? 1 : 0;
	::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&optionValue, sizeof(optionValue));
}

void TCPSocket::setReuseAddr(bool on)
{
	int optionValue = on ? 1 : 0;
	::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optionValue, sizeof(optionValue));
}

void TCPSocket::setKeepAlive(bool on)
{
	int optionValue = on ? 1 : 0;
	::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&optionValue, sizeof(optionValue));
}


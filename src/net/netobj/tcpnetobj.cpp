#include "tcpnetobj.h"
#include <thread>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include "../network/network.h"
#include "../socket/socketops.h"

TcpNetObj::TcpNetObj(uint64_t net_id, SOCKET fd):
	BaseNetObj(net_id,fd)
{
	m_net_mode = TCP;

	setNoDelay(true);
	setReuseAddr(true);
	setKeepAlive(true);
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

void TcpNetObj::setLingerZero(bool on)
{
	if (on)
	{
		struct linger opt = { 1, 0 };
		::setsockopt(m_fd, SOL_SOCKET, SO_LINGER, (const char*)&opt, sizeof(opt));
	}

	m_network->NETERROR << "TCP obj set lingerzero : " << on;
}

bool TcpNetObj::bind(InetAddress& address)
{
	BaseNetObj::bind(address);

	if (bindSocket(m_fd, address.getSockAddr(), m_error) < 0)
	{
		m_network->NETERROR << "TCP server bind fail. error:" << m_error;

		return false;
	}

	return true;
}

bool TcpNetObj::listen()
{
	BaseNetObj::listen();

	if (listenSocket(m_fd, m_error) < 0)
	{
		m_network->NETERROR << "TCP server listen fail. error:" << m_error;

		return false;
	}

	m_net_state = Listening;

	return true;
}

bool TcpNetObj::connect(InetAddress& address, uint64_t outms)
{
	m_peerAddr = address;

	if (connectSocket(m_fd, address.getSockAddr(), m_error) < 0)
	{
#ifdef _WIN32
		if (m_error == WSAEWOULDBLOCK || m_error == WSAEALREADY)
#else
		if (m_error == EINPROGRESS || m_error == EWOULDBLOCK || m_error == EAGAIN)
#endif
		{
			//等待outms后检测是否可写
			if (selectSocket(m_fd,(int64_t)outms) == 0)
			{
				return false;
			}
		}
		else
		{
			m_network->NETERROR << "TCP obj connect fail. error:" << m_error;
			return false;
		}
	}
	sockaddr_storage localaddr;
	getLocalAddr(m_fd, localaddr);
	m_localAddr = InetAddress(localaddr);

	m_net_state = Connected;
	return true;
}

bool TcpNetObj::asynConnect(InetAddress& address, uint64_t outms)
{
	(void)outms;

	m_peerAddr = address;

	if (connectSocket(m_fd, address.getSockAddr(), m_error) < 0)
	{
#ifdef _WIN32
		if (m_error == WSAEWOULDBLOCK || m_error == WSAEALREADY)
#else
		if (m_error == EINPROGRESS || m_error == EWOULDBLOCK || m_error == EAGAIN)
#endif
		{
			m_net_state = Connecting;
			return true;
		}
		else
		{
			m_network->NETERROR << "TCP server connect fail. error:" << m_error;
			return false;
		}
	}

	return false;
}

bool TcpNetObj::send(const char* data, size_t len)
{
	ssize_t write_size = 0;
	if (m_input_buf.size() == 0)
	{
		//直接写入socket
		write_size = writeSocket(m_fd, data, len, m_error);
		
		if (write_size == -1)
		{
			m_network->NETERROR << "TCP obj send fail. error:" << m_error;

			return false;
		}
	}

	//剩余部分写入input buffer;
	if (len > write_size)
	{
		std::lock_guard<std::mutex> mylockguard(m_input_mutex);
		m_input_buf.pushCString(data + write_size, len - write_size);
	}

	return true;
}

bool TcpNetObj::close()
{
	closeSocket(m_fd);
	m_fd = -1;
	m_net_state = Disconnected;

	m_network->NETERROR << "TCP obj close. net id:" << m_net_id;
	return true;
}

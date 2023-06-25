#include "udpnetobj.h"
#include "../socket/socketops.h"
#include "../network/network.h"
UDPNetObj::UDPNetObj(uint64_t net_id, SOCKET fd):
	BaseNetObj(net_id, fd)
{
	m_net_mode = (int)NetMode::UDP;
}

UDPNetObj::~UDPNetObj()
{

}

bool UDPNetObj::bind(InetAddress& address)
{
	BaseNetObj::bind(address);

	//m_network->NETERROR << "UDP server:" << address.toIpPort();

	if (bindSocket(m_fd, address.getSockAddr(),address.getSockAddrLen(), m_error) < 0)
	{
		m_network->NETERROR << "UDP server bind fail. error:" << m_error;

		return false;
	}

	return true;
}

bool UDPNetObj::connect(InetAddress& address, uint64_t outms)
{
	m_peerAddr = address;

	if (connectSocket(m_fd, address.getSockAddr(), address.getSockAddrLen(), m_error) < 0)
	{
#ifdef _WIN32
		if (m_error == WSAEWOULDBLOCK || m_error == WSAEALREADY)
#else
		if (m_error == EINPROGRESS || m_error == EWOULDBLOCK || m_error == EAGAIN)
#endif
		{
			//等待outms后检测是否可写
			if (selectSocket(m_fd, (int64_t)outms) == 0)
			{
				return false;
			}
		}
		else
		{
			m_network->NETERROR << "UDP obj connect fail. error:" << m_error;
			return false;
		}
	}
	sockaddr_storage localaddr;
	getLocalAddr(m_fd, localaddr);
	m_localAddr = InetAddress(localaddr);

	m_net_state = Connected;
	return true;
}

bool UDPNetObj::asynConnect(InetAddress& address, uint64_t outms)
{
	(void)outms;

	m_peerAddr = address;

#ifndef _WIN32
	if (connectSocket(m_fd, address.getSockAddr(), address.getSockAddrLen(), m_error) < 0)
	{
		if (m_error == EINPROGRESS || m_error == EWOULDBLOCK || m_error == EAGAIN)
		{
			haveWrite(true);
			m_net_state = Connecting;
			return true;
		}
		else
		{
			m_network->NETERROR << "TCP server connect fail. error:" << m_error;
			return false;
		}
	}
	else
	{
		m_net_state = Connected;
		return true;
	}
#else
	//对于Windows平台投递到IOCP进行异步连接
	m_net_state = Connecting;
	return true;
#endif
}

bool UDPNetObj::sendTo(InetAddress& address, const char* data, size_t len)
{
#ifndef _WIN32
	ssize_t write_size = writeToSocket(m_fd, data, len, address.getSockAddr(), address.getSockAddrLen(), m_error);

	if (write_size == -1)
	{
		if (m_error == EINPROGRESS || m_error == EWOULDBLOCK || m_error == EAGAIN)
		{
			Message msg;
			msg.m_addr = address;
			msg.m_message = std::string(data, len);
			pushMessage(msg);
			return true;
		}
		return false;
	}

#else
	Message msg;
	msg.m_addr = address;
	msg.m_message = std::string(data, len);
	pushMessage(msg);

	return true;
#endif
}

bool UDPNetObj::close()
{
	closeSocket(m_fd);
	m_fd = INVALID_SOCKET;
	m_net_state = Disconnected;
	m_network->NETDEBUG << "UDP obj close. net id:" << m_net_id;
	return true;
}

bool UDPNetObj::getMessage(Message& msg)
{
	std::lock_guard<std::mutex> lock(m_msg_mutex);
	if (m_messageQueue.empty()) return false;

	msg = m_messageQueue.front();
	m_messageQueue.pop();
	return true;
}

void UDPNetObj::pushMessage(Message& msg)
{
	std::lock_guard<std::mutex> lock(m_msg_mutex);
	m_messageQueue.push(msg);
}

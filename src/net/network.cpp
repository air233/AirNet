#include "network.h"
#include "socket/tcpsocket.h"
#include "socket/udpsocket.h"
#include "socket/kcpsocket.h"
#include "../common/log/log.h"

std::shared_ptr<BaseSocket> Network::makeSocket(SOCKET_TYPE mode)
{
	std::shared_ptr<BaseSocket> socket;

	if (mode == TCP)
	{
		socket = std::make_shared<TCPSocket>();
	}
	else if (mode == UDP)
	{
		socket = std::make_shared<UDPSocket>();
	}
	else if (mode == KCP)
	{
		socket = std::make_shared<KCPServer>();
	}
	else
	{
		LOG_FATAL << "socket type err :" << mode;
	}

	return socket;
}

Network::Network(SOCKET_TYPE mode):
	m_net_id(0), m_init(0), m_socket(Network::makeSocket(mode))
{

}

bool Network::start()
{
	if (m_init == 0)
	{
		LOG_INFO << "network not initialized!";
		return false;
	}

	return m_socket->start() == 0;
}

#include "network.h"
#include "../socket/socketops.h"
#include "../netobj/tcpnetobj.h"
//#include "../netobj/udpsocket.h"
//#include "../netobj/kcpsocket.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

std::shared_ptr<BaseNetObj> Network::makeNetObj(Network* network, sa_family_t family)
{
	std::shared_ptr<BaseNetObj> netObj;

	uint64_t netid = getNetID();

	SOCKET sock = createTCPSocket(family);

	if (network->getNetMode() == TCP)
	{
		netObj = std::make_shared<TcpNetObj>(netid, sock);
	}
	//TODO:其他Mode

	netObj->setNetwork(network);

	return netObj;
}

Network::Network(NetMode mode):
	INetWrok(mode),
	m_mode(mode),
	m_net_id(0), 
	m_init(0), 
	m_log("./log/network/","network"),
	m_idle_fd(-1),
	m_ssl(0),
	m_epoll_et(0)
{

}

Network::~Network()
{
	rlease();
}

bool Network::start()
{
	//if (m_init == 0)
	//{
	//	m_log.Debug() << "network not initialized!";
	//	return false;
	//}
#ifdef _WIN32
	WSADATA wsaData;
	int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (r != 0)
	{
		NETERROR << "network WSAStartup failed with error.";
		return false;
	}
#endif

	m_idle_fd = createFd();

	//TODO:开启工作线程

	return true;
}

void Network::update()
{
	//TODO:处理所有的socket消息 回调onReceive()
	
	//1.处理连接消息



}

void Network::stop()
{
	//TODO:关闭工作线程
}

void Network::setOpenSSL(bool bEnable)
{

}

uint64_t Network::linstenTCP(InetAddress& address, TCPServerConfig& config)
{
	if (m_mode != TCP)
	{
		return INVALID_NET_ID;
	}

	sa_family_t family = address.family();

	m_server_obj = makeNetObj(this, family);

	std::shared_ptr<TcpNetObj> tcpNetObj = std::dynamic_pointer_cast<TcpNetObj>(m_server_obj);

	if (tcpNetObj == nullptr)
	{
		return INVALID_NET_ID;
	}

	tcpNetObj->setNoDelay(config.delay);
	tcpNetObj->setReuseAddr(config.reuse);
	tcpNetObj->setKeepAlive(config.keepalive);
	tcpNetObj->setLingerZero(config.lingerzero);

	if (false == m_server_obj->bind(address))
	{
		return INVALID_NET_ID;
	}

	if (false == m_server_obj->listen())
	{
		return INVALID_NET_ID;
	}

	return m_server_obj->getNetID();


	return 1;
}

uint64_t Network::linstenTCP(std::string ip, uint16_t port, TCPServerConfig& config)
{
	InetAddress listenaddr(ip,port);

	return linstenTCP(listenaddr, config);
}

void Network::rlease()
{
	NETDEBUG << "network rlease";

#ifdef _WIN32
	WSACleanup();
#endif
}

/*保证唯一且不为0*/
uint64_t Network::getNetID()
{
	while (true)
	{
		m_net_id++;

		if (m_net_id == 0)
			m_net_id = 1;

		//确定是未使用的net_id
		if (m_net_objs.find(m_net_id) == m_net_objs.end())
		{
			break;
		}
	}

	return m_net_id;
}

uint64_t Network::linsten(InetAddress& address)
{
	sa_family_t family = address.family();

	m_server_obj = makeNetObj(this, family);

	if (false == m_server_obj->bind(address))
	{
		return INVALID_NET_ID;
	}

	if (false == m_server_obj->listen())
	{
		return INVALID_NET_ID;
	}

	return m_server_obj->getNetID();
}

uint64_t Network::linsten(std::string ip, uint16_t port)
{
	InetAddress address(ip, port);

	return linsten(address);
}

uint64_t Network::connect(InetAddress& address, uint64_t timeout)
{
	auto connect_obj = makeNetObj(this, address.family());

	if (false == connect_obj->connect(address, timeout))
	{
		return INVALID_NET_ID;
	}

	auto iter = m_net_objs.insert(make_pair(connect_obj->getNetID(), connect_obj));

	if (iter.second == false)
	{
		NETERROR << "netobj insert fail. net id:" << connect_obj->getNetID();
		return INVALID_NET_ID;
	}


	return connect_obj->getNetID();
}

uint64_t Network::connect(std::string ip, uint16_t port, uint64_t timeout)
{
	InetAddress address(ip, port);

	return connect(address, timeout);
}

uint64_t Network::asynConnect(InetAddress& address, uint64_t timeout)
{
	auto connect_obj = makeNetObj(this, address.family());
	
	if(connect_obj->asynConnect(address,timeout) == false)
	{
		return INVALID_NET_ID;
	}

	auto iter = m_net_objs.insert(make_pair(connect_obj->getNetID(), connect_obj));

	if (iter.second == false)
	{
		NETERROR << "netobj insert fail. net id:" << connect_obj->getNetID();
		return INVALID_NET_ID;
	}

	m_connecting.insert(connect_obj->getNetID());

	return connect_obj->getNetID();
}

uint64_t Network::asynConnect(std::string ip, uint16_t port, uint64_t timeout /*= 10000*/)
{
	InetAddress address(ip, port);

	return asynConnect(address, timeout);
}

bool Network::send(uint64_t net_id, const char* data, size_t size)
{
	return 0;
}

void Network::close(uint64_t net_id)
{

}

std::shared_ptr<INetObj> Network::getNetObj(uint64_t net_id)
{
	auto it = m_net_objs.find(net_id);

	if (it != m_net_objs.end())
	{
		return std::static_pointer_cast<INetObj>(it->second);
	}
	else 
	{
		return nullptr;
	}
}

NetMode Network::getNetMode()
{
	return m_mode;
}

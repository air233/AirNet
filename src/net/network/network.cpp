#include "network.h"
#include "socket/tcpsocket.h"
#include "socket/udpsocket.h"
#include "socket/kcpsocket.h"

#define NETDEBUG m_log.Debug()
#define NETINFO m_log.Error()
#define NETWARN m_log.Warn()
#define NETERROR m_log.Error()
#define NETFATAL m_log.Fatal()

std::shared_ptr<BaseSocket> Network::makeSocket(NetMode mode)
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

Network::Network(NetMode mode):
	INetWrok(mode),
	m_net_id(0), 
	m_init(0), 
	m_listen_socket(Network::makeSocket(mode)),
	m_log("./log/network/","network")
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

	//TODO:开启工作线程

	return true;
}

void Network::update()
{
	//TODO:处理所有的socket消息 回调onReceive()
}

void Network::stop()
{
	//TODO:关闭工作线程
}

void Network::rlease()
{
	NETDEBUG << "network rlease";

	//TODO:资源释放

#ifdef _WIN32
	WSACleanup();
#endif
}

uint64_t Network::getNetID()
{
	return m_net_id++;
}

uint64_t Network::linsten(InetAddress& address)
{
	return 0;
}

uint64_t Network::linsten(std::string ip, std::string port)
{
	return 0;
}

uint64_t Network::connect(InetAddress& address, uint32_t time)
{
	return 0;
}

uint64_t Network::connect(std::string ip, std::string port, uint32_t time)
{
	return 0;
}

uint64_t Network::asynConnect(InetAddress& address, uint32_t time)
{
	return 0;
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
	auto it = m_netobjs.find(net_id);
	if (it != m_netobjs.end()) 
	{
		return std::static_pointer_cast<INetObj>(it->second);
	}
	else 
	{
		return nullptr;
	}
}

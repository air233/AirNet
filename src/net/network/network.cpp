#include "network.h"
#include "../../common/until/times.h"
#include "../socket/socketops.h"
#include "../netobj/tcpnetobj.h"
//#include "../netobj/udpsocket.h"
//#include "../netobj/kcpsocket.h"

#include <thread>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

Network::Network(NetMode mode):
	INetWrok(mode),
	m_mode(mode),
	m_net_id(0), 
	m_poll(new Poll()),
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

std::shared_ptr<BaseNetObj> Network::makeNetObj(Network* network, sa_family_t family)
{
	uint64_t netid = getNetID();

	SOCKET sock = createTCPSocket(family);

	return makeNetObj(network, sock);
}

std::shared_ptr<BaseNetObj> Network::makeNetObj(Network* network, SOCKET sock)
{
	std::shared_ptr<BaseNetObj> netObj;

	uint64_t netid = getNetID();

	if (network->getNetMode() == TCP)
	{
		netObj = std::make_shared<TcpNetObj>(netid, sock);
	}
	else if (network->getNetMode() == UDP)
	{
		//TODO:其他Mode
	}
	else if (network->getNetMode() == KCP)
	{
		//TODO:其他Mode
	}
	else
	{
		NETERROR << "invalid net mode:" << network->getNetMode();
		return nullptr;
	}

	netObj->setNetwork(network);

	return netObj;
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

	return m_poll->createPoll(this);
}

void Network::update()
{
	//TODO:处理所有的socket消息 回调onReceive()
	
	//1.处理连接消息
	processAsynConnectResult();

	//2.处理文件描述符读写
	//m_poll->processPoll();

}

void Network::stop()
{
	m_poll->destoryPoll();

	closeSocket(m_idle_fd);

	m_server_obj->close();
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

	if (false == insertNetObj(m_server_obj, true))
	{
		return INVALID_NET_ID;
	}

	NETDEBUG << "listen fd:" << m_server_obj->fd() << ", linstenTCP ip : " << address.toIpPort();

	return m_server_obj->getNetID();
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

/*TODO:需要测试多线程正确性*/
/*保证唯一且不为0*/
uint64_t Network::getNetID()
{
	while (true)
	{
		m_net_id.fetch_add(1);

		if (m_net_id == 0)
		{
			bool exchange = false;
			while (!exchange)
			{
				uint64_t expectet = 0;
				exchange = m_net_id.compare_exchange_weak(expectet, 1);
			}
		}

		//未使用的net_id
		if (m_net_objs.find(m_net_id) == m_net_objs.end())
		{
			break;
		}
	}

	return m_net_id;
}

SOCKET Network::getListenSock()
{
	if (m_server_obj == nullptr)
	{
		return INVALID_SOCKET;
	}

	return m_server_obj->fd();
}

//uint64_t Network::processAccept(SOCKET sock, int32_t error)
//{
//	if (sock < 0)
//	{
//		NETERROR << "accept invalid sock. error:" << error;
//		
//#ifdef _WIN32
//		if(error == WSAEMFILE)
//#else
//		if (error == EMFILE)
//#endif
//		{
//			closeSocket(m_idle_fd);
//			//m_idle_fd = m_server_obj->accept();
//
//			m_idle_fd = createFd();
//		}
//	}
//}

std::shared_ptr<INetObj> Network::getNetObj(uint64_t net_id)
{
	std::shared_lock<std::shared_mutex> lock(m_net_mutex);

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

std::shared_ptr<BaseNetObj> Network::getNetObj2(uint64_t net_id)
{
	std::shared_lock<std::shared_mutex> lock(m_net_mutex);

	auto it = m_net_objs.find(net_id);

	if (it != m_net_objs.end())
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
}

std::shared_ptr<BaseNetObj> Network::getServerNetObj()
{
	return m_server_obj;
}

bool Network::insertNetObj(std::shared_ptr<BaseNetObj> netObj, bool addPoll)
{
	if (addPoll == true )
	{
		if (false == m_poll->addPoll(netObj))
		{
			NETERROR << "insert net obj fail." << netObj->getNetID();
			return false;
		}
	}

	std::unique_lock<std::shared_mutex> lock(m_net_mutex);
	auto iter = m_net_objs.insert(std::make_pair(netObj->getNetID(), netObj));
	return iter.second;
}

void Network::removeNetObj(uint64_t net_id)
{
	std::unique_lock<std::shared_mutex> lock(m_net_mutex);

	m_net_objs.erase(net_id);
}

void Network::deleteNetObj(uint64_t net_id)
{
	auto netObj = getNetObj2(net_id);
	if (netObj == nullptr) return;

	m_poll->delPoll(netObj);

	removeNetObj(net_id);

	NETDEBUG << "delete net obj. id:" << net_id;
}

void Network::asyncConnectResult(uint64_t net_id, int32_t err)
{
	auto net_obj = getNetObj2(net_id);
	if (net_obj == nullptr)
	{
		NETDEBUG << "net objs not find net obj:" << net_id;
		return;
	}

	std::lock_guard<std::mutex> guard(m_connect_mutex);

	if (m_connecting.count(net_id) == 0)
	{
		NETDEBUG << "connecting not find net obj:" << net_id;
		return;
	}

	net_obj->setError(err);

	m_connect_result.insert(net_id);
	m_connecting.erase(net_id);
}

void Network::processAsynConnectResult()
{
	uint64_t now = GetMSTime();
	std::set<uint64_t> connect_result;
	
	{
		std::lock_guard<std::mutex> guard(m_connect_mutex);
		connect_result.swap(m_connect_result);

		std::vector<uint64_t> delnetid;
		/*处理连接超时*/
		for (const auto& info : m_connecting)
		{
			if (info.second.m_timeout <= now)
			{
				delnetid.push_back(info.first);

				auto net_obj = getNetObj2(info.first);
				if (net_obj == nullptr)
				{
					NETDEBUG << "async m_connecting net obj not find :" << info.first;
					continue;
				}

				auto iter = connect_result.insert(info.first);
				if (iter.second == false) continue;

				net_obj->setError(NET_TIMEOUT);
			}
		}

		for (auto net_id : delnetid)
		{
			m_connecting.erase(net_id);
		}
	}
	
	/*处理连接结果*/
	for (auto net_id : connect_result)
	{
		auto net_obj = getNetObj2(net_id);

		if (net_obj == nullptr)
		{
			NETDEBUG << "async connect net obj not find :" << net_id;
			continue;
		}

		//回调连接结果
		m_onConnect(net_id, net_obj->getError());

		//如果失败，NetObj删除 net_id失效
		if (net_obj->getError() != NET_SUCCESS)
		{
			deleteNetObj(net_id);
		}
	}
}

uint64_t Network::linsten(InetAddress& address)
{
	if (m_server_obj != nullptr)
	{
		return INVALID_NET_ID;
	}

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

	m_poll->addPoll(m_server_obj);

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

	if (insertNetObj(connect_obj, true) == false)
	{
		NETERROR << "net obj insert fail. net id:" << connect_obj->getNetID();

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

	if (insertNetObj(connect_obj) == false)
	{
		NETERROR << "net obj insert fail. net id:" << connect_obj->getNetID();
		return INVALID_NET_ID;
	}

	ConnectInfo stConnectInfo;
	stConnectInfo.m_net_id = connect_obj->getNetID();
	stConnectInfo.m_timeout = GetMSTime() + timeout;
	NETDEBUG << "asynConnect time out:" << GetMSTimeStr(stConnectInfo.m_timeout);
	m_connecting.insert(std::make_pair(connect_obj->getNetID(), stConnectInfo));

	return connect_obj->getNetID();
}

uint64_t Network::asynConnect(std::string ip, uint16_t port, uint64_t timeout /*= 10000*/)
{
	InetAddress address(ip, port);

	return asynConnect(address, timeout);
}

bool Network::send(uint64_t net_id, const char* data, size_t size)
{
	NETERROR << "send :" << net_id;

	auto netObj = getNetObj2(net_id);

	if (netObj == nullptr)
	{
		NETERROR << "invalid net id :" << net_id;
		return false;
	}

	bool ret = netObj->send(data, size);

	if (ret == false)
	{
		close(net_id);
	}

	return ret;
}

void Network::close(uint64_t net_id)
{
	auto netObj = getNetObj2(net_id);

	if (netObj == nullptr)
	{
		NETERROR << "invalid net id :" << net_id;
		return;
	}

	netObj->close();

	/*回调断开连接 此时还能拿到对象*/
	m_onDisconnect(net_id);

	/*释放netobj 对象消失*/
	deleteNetObj(net_id);
}



NetMode Network::getNetMode()
{
	return m_mode;
}

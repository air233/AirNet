#include "network.h"
#include "../../common/until/times.h"
#include "../socket/socketops.h"
#include "../netobj/tcpnetobj.h"
#include "../netobj/udpnetobj.h"
//#include "../netobj/kcpnetobj.h"

#include <thread>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

Network::Network(NetMode mode):
	INetWrok(mode),
	m_run(0),
	m_mode(mode),
	m_net_id(0), 
	m_poll(new Poll()),
	m_log("./log/network/","network"),
	m_idle_fd(-1),
	m_ssl(0),
	m_epoll_et(0),
	m_server_net_id(INVALID_NET_ID)
{
	
}

Network::~Network()
{
	if (m_run)
	{
		stop();
	}

	rlease();
}

std::shared_ptr<BaseNetObj> Network::makeNetObj(Network* network, sa_family_t family)
{
	SOCKET sock = INVALID_SOCKET;

	if (network->getNetMode() == (int)NetMode::TCP)
	{
		sock = createTCPSocket(family);
	}
	else if (network->getNetMode() == (int)NetMode::UDP)
	{
		sock = createUDPSocket(family);
	}
	else
	{
		NETERROR << "invalid net mode:" << network->getNetMode();
		return nullptr;
	}

	return makeNetObj(network, sock);
}

std::shared_ptr<BaseNetObj> Network::makeNetObj(Network* network, SOCKET sock)
{
	std::shared_ptr<BaseNetObj> netObj;

	uint64_t netid = getNextNetID();

	if (network->getNetMode() == (int)NetMode::TCP)
	{
		netObj = std::make_shared<TCPNetObj>(netid, sock);
	}
	else if (network->getNetMode() == (int)NetMode::UDP)
	{
		netObj = std::make_shared<UDPNetObj>(netid, sock);
	}
	else if (network->getNetMode() == (int)NetMode::KCP)
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
	NETINFO << "[network] start.";

	bool ret = false;
#ifdef _WIN32
	WSADATA wsaData;
	int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (r != 0)
	{
		NETERROR << "network WSAStartup failed with error.";
		return false;
	}
#else
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
#endif
	m_idle_fd = createFd();

	ret = m_poll->createPoll(this);

	m_run = 1;

	
	return ret;
}

void Network::update()
{
	//1.处理异步连接超时
	processAsynConnectTimeOut();

	//2.处理IO产生的Job
	m_poll->processJob();
}

void Network::stop()
{
	m_poll->destoryPoll();

	if (m_idle_fd != INVALID_SOCKET)
	{
		closeSocket(m_idle_fd);
	}

	if (m_server_obj != nullptr)
	{
		m_server_obj->close();
	}

	m_run = 0;
	NETINFO << "[network] stop.";
}

void Network::setOpenSSL(bool bEnable)
{

}

uint64_t Network::linstenTCP(InetAddress& address, TCPServerConfig& config)
{
	if (m_mode != NetMode::TCP)
	{
		return INVALID_NET_ID;
	}

	sa_family_t family = address.family();

	m_server_obj = makeNetObj(this, family);

	std::shared_ptr<TCPNetObj> tcpNetObj = std::dynamic_pointer_cast<TCPNetObj>(m_server_obj);

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

	if (false == insertNetObj(m_server_obj))
	{
		return INVALID_NET_ID;
	}

	NETDEBUG << "listen fd:" << m_server_obj->fd() << ", linstenTCP ip : " << address.toIpPort();

	m_server_net_id = m_server_obj->getNetID();

	return m_server_net_id;
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
uint64_t Network::getNextNetID()
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
		//将IO事件加入Poll层
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

	NETDEBUG << "delete net obj. id:" << net_id;
	
	netObj->close();/*网络对象关闭*/
	
	m_poll->delPoll(netObj);

	removeNetObj(net_id);

	if (m_server_net_id == net_id)
	{
		NETWARN << "SERVER CORE CLOSE!";
	}
}

void Network::asyncConnectResult(uint64_t net_id, int32_t err)
{
	auto net_obj = getNetObj2(net_id);
	if (net_obj == nullptr)
	{
		NETDEBUG << "net objs not find net obj:" << net_id;
		return;
	}

	net_obj->setError(err);

	if (err == NET_SUCCESS)
	{
		net_obj->setNetStatus(Connected);
	}
	else
	{
		net_obj->setNetStatus(Disconnected);
	}
	
	std::lock_guard<std::mutex> guard(m_connect_mutex);
	if (m_connecting.count(net_id) == 0)
	{
		NETDEBUG << "connecting not find net obj:" << net_id;
		return;
	}
	
	m_connecting.erase(net_id);
}

void Network::processAsynConnectTimeOut()
{
	uint64_t now = GetMSTime();
	/*处理连接超时*/
	std::set<uint64_t> connect_result;
	{
		std::lock_guard<std::mutex> guard(m_connect_mutex);
		std::vector<uint64_t> delnetid;
		
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
	
	for (auto net_id : connect_result)
	{
		auto net_obj = getNetObj2(net_id);

		if (net_obj == nullptr)
		{
			NETDEBUG << "async connect net obj not find :" << net_id;
			continue;
		}

		//超时处理
		m_onConnect(net_id, NET_TIMEOUT);

		deleteNetObj(net_id);
	}
}

uint64_t Network::connect(InetAddress& address, uint64_t timeout)
{
	auto connect_obj = makeNetObj(this, address.family());

	if (false == connect_obj->connect(address, timeout))
	{
		return INVALID_NET_ID;
	}

	if (insertNetObj(connect_obj) == false)
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

uint64_t Network::bindUDP(InetAddress& address)
{
	if (m_mode != NetMode::UDP)
	{
		return INVALID_NET_ID;
	}

	sa_family_t family = address.family();

	m_server_obj = makeNetObj(this, family);

	if (false == m_server_obj->bind(address))
	{
		return INVALID_NET_ID;
	}

	NETDEBUG << "UDPServer fd:" << m_server_obj->fd() << ", bind address : " << address.toIpPort();

	if (false == insertNetObj(m_server_obj))
	{
		return INVALID_NET_ID;
	}

	m_server_net_id = m_server_obj->getNetID();
		
	return m_server_net_id;
}

uint64_t Network::bindUDP(std::string ip, uint16_t port)
{
	InetAddress address(ip, port);

	return bindUDP(address);
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
	m_connecting.insert(std::make_pair(connect_obj->getNetID(), stConnectInfo));
	//NETDEBUG << "asynConnect time out:" << GetMSTimeStr(stConnectInfo.m_timeout);

	return connect_obj->getNetID();
}

uint64_t Network::asynConnect(std::string ip, uint16_t port, uint64_t timeout /*= 10000*/)
{
	InetAddress address(ip, port);

	return asynConnect(address, timeout);
}

bool Network::send(uint64_t net_id, const char* data, size_t size)
{
	auto netObj = getNetObj2(net_id);

	if (netObj == nullptr)
	{
		NETERROR << "invalid net id :" << net_id;
		return false;
	}

	bool ret = netObj->send(data, size);

	if (ret == false)
	{
		m_onError(net_id, netObj->getError());
		return false; 
	}

	//开启监听写事件
	ret = m_poll->enableWritePoll(netObj, true);

	return ret;
}

bool Network::sendTo(InetAddress& address, const char* data, size_t size)
{
	std::string message(data, size);

	bool ret = m_server_obj->sendTo(address, data, size);

	ret = m_server_obj->sendTo(address, data, size);

	//开启监听写事件
	ret = m_poll->enableWritePoll(m_server_obj, true);

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

	/*回调断开连接 此时还能拿到对象*/
	m_onDisconnect(net_id);

	/*释放netobj 对象消失*/
	deleteNetObj(net_id);
}

void Network::disconnect(uint64_t net_id)
{
	auto netObj = getNetObj2(net_id);

	if (netObj == nullptr)
	{
		NETERROR << "invalid net id :" << net_id;
		return;
	}

	deleteNetObj(net_id);
}

int Network::getNetMode()
{
	return (int)m_mode;
}

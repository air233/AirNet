#include <iostream>
#include "../net/inetwork.h"
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32
#include <functional>
#include "../common/until/times.h"
#include "../common/buffer/buffer.h"
#include "../net/network/network.h"
#include <thread>

class MyServer
{
public:
	std::shared_ptr<INetWrok> m_network;
	void ConnectCallback(const uint64_t, int32_t);
	void Disconnected(const uint64_t);
	void Receive(const uint64_t net_id, Buffer* buff);
	void AccpetCallback(const uint64_t net_id);
};



void MyServer::ConnectCallback(const uint64_t net_id, int32_t error)
{
	std::cout << net_id << " connect result:" << error << ", time:" << GetMSTimeStr() << std::endl;

	auto netObj = m_network->getNetObj(net_id);

	if (netObj)
	{
		std::cout << "connect success. local addr:" << netObj->localAddress().toIpPort() << ". peer addr:" << netObj->peerAddress().toIpPort() << std::endl;
	}

	//m_network->send(net_id, "hello", 5);
}

void MyServer::Disconnected(const uint64_t net_id)
{
	std::cout << net_id << " disconnect:"<< GetMSTimeStr() << std::endl;
}

void MyServer::Receive(const uint64_t net_id, Buffer* buff)
{
	std::string str;

	buff->peekString(str, buff->size());

	std::cout << net_id <<",size:" << str.size() << " recv:" << str << std::endl;

	m_network->send(net_id, str.c_str(), str.size());
}

void MyServer::AccpetCallback(const uint64_t net_id)
{
	std::cout << "AccpetID:" << net_id << std::endl;

	m_network->send(net_id, "hello", 5);
}

int test_server()
{
	MyServer server;
	server.m_network = getNetwork(NetMode::TCP);
	//�ص�����
	server.m_network->setConnectCallback(std::bind(&MyServer::ConnectCallback, &server, std::placeholders::_1, std::placeholders::_2));
	server.m_network->setDisConnectCallback(std::bind(&MyServer::Disconnected, &server, std::placeholders::_1));
	server.m_network->setRecvCallback(std::bind(&MyServer::Receive, &server, std::placeholders::_1, std::placeholders::_2));
	server.m_network->setAcceptCallback(std::bind(&MyServer::AccpetCallback, &server, std::placeholders::_1));
	//����������
	server.m_network->start();

	//std::cout << "asynConnect:" << GetMSTimeStr() << std::endl;
	//auto net_id = server.m_network->asynConnect("192.168.2.161", 1301, 5000);
	//std::cout << "connect id :" << net_id << std::endl;

	TCPConfig config = { 0,0,0 };
	auto net_id2 = server.m_network->linstenTCP("0.0.0.0", 8888, config);
	std::cout << "listen id :" << net_id2 << std::endl;

	while (true)
	{
		server.m_network->update();

		Sleep(100);
	}

	server.m_network->stop();
	return 0;
}

int test_getNetID()
{
	std::shared_ptr<INetWrok> server = getNetwork(NetMode::TCP);

	std::shared_ptr<Network> networkServer = std::dynamic_pointer_cast<Network>(server);

	std::vector<std::thread> oThreads;
	for (int i = 0; i < 5; i++)
	{
		oThreads.push_back(std::thread([networkServer]() {
			for (int i = 0; i < 1000000; i++)
			{
				//std::cout << "thread: " << networkServer->getNetID() << std::endl;
				networkServer->getNextNetID();
			}
		}
		));
	}

	for (auto& therda : oThreads)
	{
		therda.join();
	}

	std::cout << "Final net id: " << networkServer->getNextNetID() << std::endl;

	return 0;
}



int main()
{
	test_server();

	return 0;
}
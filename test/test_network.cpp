#include <iostream>
#include "../net/inetwork.h"
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32
#include <functional>
#include "../common/until/times.h"


class MyServer
{
public:
	std::shared_ptr<INetWrok> m_network;
	void ConnectCallback(uint64_t, int32_t);
	void Disconnected(uint64_t);
};

void MyServer::ConnectCallback(uint64_t net_id, int32_t error)
{
	std::cout << net_id << " connect result:" << error << "," << GetMSTimeStr() << std::endl;

	if (error == 0)
	{
		//连接成功后关闭
		m_network->close(net_id);
	}
}

void MyServer::Disconnected(uint64_t net_id)
{
	std::cout << net_id << " disconnect:"<< GetMSTimeStr() << std::endl;
}

int main()
{
	MyServer server;
	server.m_network = getNetwork(TCP);
	//回调函数
	server.m_network->setConnectCallback(std::bind(&MyServer::ConnectCallback, &server, std::placeholders::_1, std::placeholders::_2));
	server.m_network->setDisConnectCallback(std::bind(&MyServer::Disconnected, &server, std::placeholders::_1));
	//启动
	server.m_network->start();
	
	std::cout <<"asynConnect:"<< GetMSTimeStr() << std::endl;

	auto net_id = server.m_network->asynConnect("192.168.2.161", 1301,5000);
	std::cout << "connect id :" << net_id << std::endl;

	TCPConfig config = {0,0,0};
	auto net_id2 = server.m_network->linstenTCP("127.0.0.1",8888, config);
	std::cout << "listen id :" << net_id2 << std::endl;

	while (true)
	{
		server.m_network->update();

		Sleep(100);
	}

	server.m_network->stop();
	return 0;
}

#include <iostream>
#include "../net/inetwork.h"
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32
int main()
{
	std::shared_ptr<INetWrok> network2;
	{
		std::shared_ptr<INetWrok> network = getNetwork(TCP);
		network2 = network;
	}

	network2->start();

	//auto net_id = network2->connect("192.168.2.161", 1301);
	//std::cout << "connect id :" << net_id << std::endl;

	TCPConfig config = {0,0,0};

	auto net_id = network2->linstenTCP("127.0.0.1",8888, config);

	std::cout << "listen id :" << net_id << std::endl;

	while (true)
	{
		network2->update();

		Sleep(1000);
	}

	return 0;
}

#include <iostream>
#include "../net/inetwork.h"

int main()
{
	std::shared_ptr<INetWrok> network2;
	{
		std::shared_ptr<INetWrok> network = getNetwork(TCP);
		network2 = network;
	}
	
	

	return 0;
}

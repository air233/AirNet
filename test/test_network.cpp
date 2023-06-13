#include <iostream>
#include "../net/inetwork.h"

int main()
{
	std::shared_ptr<INetWrok> network2;
	{
		std::shared_ptr<INetWrok> network = getNetwork2(TCP);
		network2 = network;
	}
	
	

	return 0;
}

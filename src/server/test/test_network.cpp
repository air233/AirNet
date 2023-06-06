
#include<iostream>
#include "net/network.h"

int main()
{
	std::cout << "server test" << std::endl;

	Network tcpserver;

	tcpserver.start();

	return 0;
}
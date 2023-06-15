#include <iostream>
#include "../../../common/until/encoding.h"
#include "../socketops.h"
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

//客户端
int main1()
{
	SOCKET localfd = createFd();
	std::cout << "localfd:" << localfd << std::endl;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Failed to initialize Winsock." << std::endl;
		return 1;
	}

	std::cout << "main" << std::endl;

	SOCKET fd = createTCPSocket(AF_INET);
	std::cout << "SOCKET:" << fd << std::endl;

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("192.168.2.161");
	serverAddress.sin_port = htons(1301); // 目标服务器端口
	int ret,error = 0;

	ret = 0;
	//while (true)
	//{
	//	ret = connectSocket(fd, (sockaddr*)&serverAddress, error);

	//	if (ret != 0)
	//	{
	//		std::cout << "SOCKET error:" << error << std::endl;

	//		if(error == WSAEWOULDBLOCK || error == WSAEALREADY) continue;
	//		
	//		if (error == WSAEISCONN)
	//		{
	//			ret = 0;
	//		}

	//		break;
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}

	ret = SOCKET_ERROR;
	if (connectSocket(fd, (sockaddr*)&serverAddress, error) < 0)
	{
		std::cout << "connectSocket . error:" << error << std::endl;
		if (error == WSAEWOULDBLOCK || error == WSAEALREADY)
		{
			//fd_set fds;
			//FD_ZERO(&fds);
			//FD_SET(fd, &fds);
			//timeval timeout;
			//timeout.tv_sec = 10;

			//int n = select(fd+1,NULL, &fds,NULL, &timeout);
			//std::cout << " select n:" << n << std::endl;
			//if (n != 0)
			//{
			//	std::cout << "connectSocket . select succ:" << n << std::endl;
			//	ret = 1;
			//}
			int n = selectSocket(fd, 10);

			if (n != 0)
			{
				std::cout << "connectSocket . select succ:" << n << std::endl;
				ret = 1;
			}
		}
	}


	if (ret != SOCKET_ERROR)
	{
		struct sockaddr_in localAddress;
		ret = getLocalAddr(fd, localAddress);
		std::cout << "getLocalAddr ret error:" << ret << std::endl;
		std::cout << "getLocalAddr ip:" << localAddress.sin_addr.s_addr << std::endl;
		std::cout << "getLocalAddr port:" << BigEndianToHost(localAddress.sin_port) << std::endl;

		struct sockaddr_in peerAddress;
		ret = getPeerAddr(fd, peerAddress);
		std::cout << "getPeerAddr ret error:" << ret << std::endl;
		std::cout << "getPeerAddr ip:" << peerAddress.sin_addr.s_addr << std::endl;
		std::cout << "getPeerAddr port:" << BigEndianToHost(peerAddress.sin_port) << std::endl;

		closesocket(fd);
	}

	WSACleanup();

	return 0;
}

int pp(struct sockaddr* addr)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof * addr);

	std::cout << "addrlen:" << addrlen << std::endl;

	return addrlen;
}

//服务器
int main2()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Failed to initialize Winsock." << std::endl;
		return 1;
	}

	std::cout << "main" << std::endl;

	SOCKET server_fd = createTCPSocket(AF_INET);

	int error = 0;
	
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("0.0.0.0");
	address.sin_port = htons(8888); // 服务器端口

	std::cout << "address len:" << sizeof address << std::endl;
	if (bindSocket(server_fd, (sockaddr*)&address, error) < 0)
	{
		std::cout << "bindSocket fail. error:"<< error << std::endl;
		return -1;
	}

	if (listenSocket(server_fd, error) < 0)
	{
		std::cout << "listenSocket fail. error:" << error << std::endl;
		return -1;
	}

	error = 11111;
	while (true)
	{
		struct sockaddr_in client_addres;

		SOCKET client_fd = acceptSocket(server_fd, (sockaddr*)&client_addres, error);

		if (client_fd == INVALID_SOCKET)
		{
			if (WSAEWOULDBLOCK != error)
			{
				std::cout << "acceptSocket fail. error:" << error << std::endl;
			}
			Sleep(1000);
			continue;
		}

		std::cout << "acceptSocket succ. client_fd:" << client_fd << std::endl;

		Sleep(1000);
	}

	//pp((sockaddr*)&address);

	WSACleanup();
	return 0;
}

int main()
{
	main1();

	return 0;
}
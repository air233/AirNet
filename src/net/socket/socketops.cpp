#include "socketops.h"

bool setNonBlock(SOCKET sockfd)
{
#ifdef _WIN32
	unsigned long nonBlocking = 1;
	int ret = ioctlsocket(sockfd, FIONBIO, &nonBlocking);
	return ret == 0;
#else
	int flags = ::fcntl(sockfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int ret = ::fcntl(sockfd, F_SETFL, flags);
	return ret >= 0
#endif
}

SOCKET createFd()
{
	SOCKET sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	
	return sockfd;
}

SOCKET createTCPSocket(sa_family_t family)
{
	SOCKET sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);

	if (false == setNonBlock(sockfd))
	{
		closeSocket(sockfd);
		return INVALID_SOCKET;
	}

	return sockfd;
}

SOCKET createUDPSocket(sa_family_t family)
{
	SOCKET sockfd = ::socket(family, SOCK_DGRAM, IPPROTO_UDP);

	if (false == setNonBlock(sockfd))
	{
		closeSocket(sockfd);
		return INVALID_SOCKET;
	}

	return sockfd;
}

void closeSocket(SOCKET sockfd)
{
#ifdef _WIN32
	closesocket(sockfd);
#else
	::close(sockfd);
#endif
}

int32_t getLocalAddr(SOCKET sockfd, sockaddr_in& localaddr)
{
	int error = 0;

	memset(&localaddr, 0, sizeof localaddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);

	if (::getsockname(sockfd, (sockaddr*)&localaddr, &addrlen) < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}

	return error;
}

int32_t getLocalAddr6(SOCKET sockfd, sockaddr_in6& localaddr)
{
	int error = 0;

	memset(&localaddr, 0, sizeof localaddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);

	if (::getsockname(sockfd, (sockaddr*)&localaddr, &addrlen) < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}

	return error;
}

int32_t getLocalAddr(SOCKET sockfd, sockaddr_storage& localaddr)
{
	int error = 0;
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);

	if (::getsockname(sockfd, (sockaddr*)&localaddr, &addrlen) < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}
	return error;
}

int32_t getPeerAddr(SOCKET sockfd, sockaddr_in& peeraddr)
{
	int error = 0;
	memset(&peeraddr, 0, sizeof peeraddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);

	if (::getpeername(sockfd, (sockaddr*)&peeraddr, &addrlen) < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}

	return error;
}

int32_t getPeerAddr6(SOCKET sockfd, sockaddr_in6& peeraddr)
{
	int error = 0;
	memset(&peeraddr, 0, sizeof peeraddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);

	if (::getpeername(sockfd, (sockaddr*)&peeraddr, &addrlen) < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}

	return error;
}

int32_t getPeerAddr(SOCKET sockfd, sockaddr_storage& peeraddr)
{
	int error = 0;
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);

	if (::getpeername(sockfd, (sockaddr*)&peeraddr, &addrlen) < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}
	return error;
}

/*
Linux:EMFILE
Windows:WSAEMFILE
*/
SOCKET acceptSocket(SOCKET sockfd, struct sockaddr* addr, int32_t& error)
{
	socklen_t addrlen = static_cast<socklen_t>(sizeof * addr);

	SOCKET connfd = ::accept(sockfd, addr, &addrlen);

#ifdef _WIN32
	if (connfd == INVALID_SOCKET)
#else
	if (connfd < 0)
#endif
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
		return INVALID_SOCKET;
	}

	if (false == setNonBlock(connfd))
	{
		return INVALID_SOCKET;
	}

	error = 0;

	return connfd;
}

int32_t connectSocket(SOCKET sockfd, struct sockaddr* addr, int32_t& error)
{
	error = 0;

	socklen_t addrlen = static_cast<socklen_t>(sizeof * addr);
	int ret = ::connect(sockfd, addr, addrlen);

	if (ret < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}

	return ret;
}

int32_t bindSocket(SOCKET sockfd, struct sockaddr* addr, int32_t& error)
{
	error = 0;

	socklen_t addrlen = static_cast<socklen_t>(sizeof * addr);
	int ret = ::bind(sockfd, addr, addrlen);

	if (ret < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}

	return ret;
}

bool listenSocket(SOCKET sockfd)
{
	int ret = ::listen(sockfd, SOMAXCONN);

	return ret >= 0;
}

int32_t listenSocket(SOCKET sockfd, int32_t& error)
{
	error = 0;

	int ret = ::listen(sockfd, SOMAXCONN);

	if (ret < 0)
	{
#ifdef _WIN32
		error = WSAGetLastError();
#else
		error = errno;
#endif
	}

	return ret;
}

//int32_t connectSocket(SOCKET sockfd, const struct sockaddr* addr)
//{
//
//}

//int32_t connect(SOCKET sockfd, const struct sockaddr* addr)
//{
//	return ::connect(sockfd, addr, (socklen_t)(sizeof sockaddr_in6));
//}

//int32_t connect(SOCKET sockfd, const struct sockaddr_in* addr4)
//{
//	return ::connect(sockfd, addr4, (socklen_t)(sizeof sockaddr_in));
//}
//
//int32_t connect(SOCKET sockfd, const struct sockaddr_in6* addr6)
//{
//	return ::connect(sockfd, addr6, (socklen_t)(sizeof sockaddr_in6));
//}
//
//bool bind(SOCKET sockfd, const struct sockaddr* addr)
//{
//	int ret = ::bind(sockfd, addr, (socklen_t)(sizeof sockaddr_in6));
//	
//	return ret >= 0;
//}
//
//bool listen(SOCKET sockfd)
//{
//	int ret = ::listen(sockfd, SOMAXCONN);
//	
//	return ret >= 0;
//}




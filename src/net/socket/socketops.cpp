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

void shutdownSocketWrite(SOCKET sockfd)
{
#ifdef _WIN32
	shutdown(sockfd, SD_SEND);
#else
	shutdown(sockfd, SHUT_WR);
#endif
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

//不可读不可写 0
//只可读 1
//只可写 2
//可读可写 3
int32_t selectSocket(SOCKET sockfd, int64_t timeout)
{
	int ret = 0;

	timeval times;
	times.tv_usec = timeout * 1000;

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(sockfd, &writefds);

	if (select(sockfd+1, &readfds, &writefds, NULL, &times) > 0)
	{
		if (FD_ISSET(sockfd, &readfds))
		{
			ret |= 1 << 0;
		}

		if (FD_ISSET(sockfd, &writefds))
		{
			ret |= 1 << 1;
		}
	}

	return ret;
}

ssize_t writeSocket(SOCKET sockfd, const void* buf, size_t count)
{
#ifdef _WIN32
	return ::send(sockfd, (const char*)buf, count, 0);
#else
	return ::write(sockfd, buf, count);
#endif
}

ssize_t readSocket(SOCKET sockfd, void* buf, size_t count)
{
#ifdef _WIN32
	return ::recv(sockfd, (char*)buf, count, 0);
#else
	return ::read(sockfd, buf, count);
#endif
}


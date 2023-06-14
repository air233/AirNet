#include "../nettype.h"
#ifdef _WIN32
#include <winsock2.h>
#include<WS2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

/*Socket 操作*/
bool setNonBlock(SOCKET sockfd);

SOCKET createFd();
SOCKET createTCPSocket(sa_family_t family);
SOCKET createUDPSocket(sa_family_t family);

SOCKET acceptSocket(SOCKET sockfd, struct sockaddr* addr, int32_t& error);

int32_t listenSocket(SOCKET sockfd, int32_t& error);

int32_t connectSocket(SOCKET sockfd, struct sockaddr* addr, int32_t& error);

int32_t bindSocket(SOCKET sockfd, struct sockaddr* addr, int32_t& error);

void closeSocket(SOCKET sockfd);

int32_t getLocalAddr(SOCKET sockfd, sockaddr_storage& localaddr);
int32_t getLocalAddr(SOCKET sockfd, sockaddr_in& localaddr);
int32_t getLocalAddr6(SOCKET sockfd, sockaddr_in6& localaddr);

int32_t getPeerAddr(SOCKET sockfd, sockaddr_storage& peeraddr);
int32_t getPeerAddr(SOCKET sockfd, sockaddr_in& peeraddr);
int32_t getPeerAddr6(SOCKET sockfd, sockaddr_in6& peeraddr);


//
//
//ssize_t read(SOCKET sockfd, void* buf, size_t count);
//
////TODO:Windows 是否支持
//ssize_t readv(SOCKET sockfd, const struct iovec* iov, int iovcnt);
//
//ssize_t write(SOCKET sockfd, const void* buf, size_t count);
//
//void shutdownWrite(SOCKET sockfd);
//
//int getSocketError(SOCKET sockfd);


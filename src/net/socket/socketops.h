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

/*Socket ����*/

bool setNonBlock(SOCKET sockfd);

SOCKET createFd();
SOCKET createTCPSocket(sa_family_t family);
SOCKET createUDPSocket(sa_family_t family);

SOCKET acceptSocket	(SOCKET sockfd, struct sockaddr* addr, int32_t addrlen, int32_t& error);

int32_t listenSocket(SOCKET sockfd, int32_t& error);

int32_t connectSocket(SOCKET sockfd, struct sockaddr* addr, int32_t addrlen, int32_t& error);

int32_t bindSocket(SOCKET sockfd, struct sockaddr* addr, int32_t addrlen, int32_t& error);

void shutdownSocketWrite(SOCKET sockfd);
void closeSocket(SOCKET sockfd);

int32_t getLocalAddr(SOCKET sockfd, sockaddr_storage& localaddr);
int32_t getLocalAddr(SOCKET sockfd, sockaddr_in& localaddr);
int32_t getLocalAddr6(SOCKET sockfd, sockaddr_in6& localaddr);

int32_t getPeerAddr(SOCKET sockfd, sockaddr_storage& peeraddr);
int32_t getPeerAddr(SOCKET sockfd, sockaddr_in& peeraddr);
int32_t getPeerAddr6(SOCKET sockfd, sockaddr_in6& peeraddr);

/*����Ƿ�ɶ���д*/
int32_t selectSocket(SOCKET sockfd, int64_t timeout);

ssize_t writeSocket(SOCKET sockfd, const void* buf, size_t count, int32_t& error);
ssize_t readSocket(SOCKET sockfd, void* buf, size_t count, int32_t& error);

ssize_t writeToSocket(SOCKET sockfd, const void* buf, size_t count,
	const struct sockaddr* dest_addr, socklen_t addrlen, int32_t& error);

ssize_t readFromSocket(SOCKET sockfd, void* buf, size_t count, 
	struct sockaddr* dest_addr, socklen_t* addrlen, int32_t& error);


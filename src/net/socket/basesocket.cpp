#include "basesocket.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <errno.h>
#endif


BaseSocket::BaseSocket():
	m_netid(0), 
	m_fd(-1), 
	m_socket_type(NONE),
	m_error(0)
{

}

BaseSocket::~BaseSocket()
{

}

//int BaseSocket::getSocketEror()
//{
//#ifdef _WIN32
//	return WSAGetLastError();
//#else
//	return errno;
//#endif
//}

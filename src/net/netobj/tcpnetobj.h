#pragma once
#include "basenetobj.h"

class TcpNetObj : public BaseNetObj
{
public:
	TcpNetObj(uint64_t net_id, SOCKET fd);
	~TcpNetObj();

	void setNoDelay(bool on);
	void setReuseAddr(bool on);
	void setKeepAlive(bool on);

	//TCPSocket m_sokcet_obj;
};

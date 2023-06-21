#pragma once
#include "basenetobj.h"

class TCPNetObj : public BaseNetObj
{
public:
	TCPNetObj(uint64_t net_id, SOCKET fd);
	~TCPNetObj();

	void setNoDelay(bool on);
	void setReuseAddr(bool on);
	void setKeepAlive(bool on);
	void setLingerZero(bool on);

	bool bind(InetAddress& address) override;
	bool listen() override;
	bool connect(InetAddress& address, uint64_t outms) override;
	bool asynConnect(InetAddress& address, uint64_t outms) override;
	bool send(const char* data, size_t len) override;
	bool close() override;
};

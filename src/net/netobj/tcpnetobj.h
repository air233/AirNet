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
	void setLingerZero(bool on);

	bool bind(InetAddress& address) override;
	bool listen() override;
	bool connect(InetAddress& address, uint64_t outms) override;
	bool asynConnect(InetAddress& address, uint64_t outms) override;
	bool send(const char* data, size_t len) override;
	bool close() override;
};

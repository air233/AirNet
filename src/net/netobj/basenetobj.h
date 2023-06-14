#pragma once

#include "../inetwork.h"
#include "../until/inetAddress.h"
#include "../../common/buffer/buffer.h"

class BaseNetObj : public  INetObj
{
public:
	BaseNetObj(uint64_t net_id, SOCKET fd);
	virtual ~BaseNetObj() {};

	uint64_t getNetID() override;
	uint32_t getNetMode() override;
	uint32_t getNetStatus() override;

	const InetAddress& localAddress() override;
	const InetAddress& peerAddress() override;

	virtual Buffer* inputBuffer() override;
	virtual Buffer* outputBuffer() override;

	SOCKET fd() override;
	int32_t getError() override;

	//���½ӿڲ���¶:
	bool connect(InetAddress& address,uint64_t outms);
	bool listen(InetAddress& address);
	void send(const char* data, size_t len);
	bool isListen();

	void setlocalAddress(InetAddress& localAddr);
	void setpeerAddress(InetAddress& peerAddr);
protected:
	uint64_t m_net_id;
	SOCKET m_fd;
	int32_t m_error;
	uint32_t m_net_mode;
	uint32_t m_net_state;
	uint8_t m_listen;

	InetAddress m_localAddr;
	InetAddress m_peerAddr;

	Buffer m_input_buf;
	Buffer m_output_buf;
};

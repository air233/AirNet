#pragma once

#include "../inetwork.h"
#include "../until/inetAddress.h"
#include "../../common/buffer/buffer.h"

#include <mutex>

class Network;

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
	void setError(int32_t error);
public:
	//以下接口不暴露:
	virtual bool bind(InetAddress& address);
	virtual bool listen();
	virtual bool connect(InetAddress& address,uint64_t outms);
	virtual bool asynConnect(InetAddress& address, uint64_t outms);
	virtual bool send(const char* data, size_t len);
	virtual bool isListen();
	virtual bool close();

	//virtual SOCKET accept(int32_t& err);

	void setlocalAddress(InetAddress& localAddr);
	void setpeerAddress(InetAddress& peerAddr);
	void setNetwork(Network* network);
protected:
	
	uint64_t m_net_id;
	SOCKET m_fd;
	int32_t m_error;
	uint32_t m_net_mode;
	uint32_t m_net_state;
	uint8_t m_listen;

	InetAddress m_localAddr;
	InetAddress m_peerAddr;

	std::mutex m_input_mutex;
	Buffer m_input_buf;

	std::mutex m_output_mutex;
	Buffer m_output_buf;

	Network* m_network;
};


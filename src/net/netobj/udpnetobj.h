#pragma once
#include "basenetobj.h"

#include <queue>



class UDPNetObj : public BaseNetObj
{
public:
	UDPNetObj(uint64_t net_id, SOCKET fd);
	~UDPNetObj();

	bool bind(InetAddress& address) override;
	bool connect(InetAddress& address, uint64_t outms) override;
	bool asynConnect(InetAddress& address, uint64_t outms) override;
	bool sendTo(InetAddress& address, const char* data, size_t len) override;
	bool close() override;
	
	bool getMessage(Message& msg) override;
	size_t getMessageSize() override;
	void pushMessage(Message& msg);

	std::mutex m_msg_mutex;
	std::queue<Message> m_messageQueue;
};
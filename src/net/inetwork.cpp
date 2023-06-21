#include "inetwork.h"
#include "network/network.h"
#include "../common/log/log.h"
#include "socket/socketops.h"

void defaultNewConnectCallback(uint64_t net_id)
{
	LOG_DEBUG << "accepted socket : " << net_id;
}

void defaultConnectCallback(uint64_t net_id, int32_t errcode)
{
	LOG_DEBUG << "connected socket : " << net_id << ". err:" << errcode;
}

void defaultDisConnectCallback(uint64_t net_id)
{
	LOG_DEBUG << "disconnected socket : " << net_id;
}

void defaultReceiveCallback(const uint64_t net_id, Buffer* buf)
{
	LOG_DEBUG << "received socket : " << net_id;
	
	buf->dropAll();
}

void defaultReceiveFromCallback(InetAddress& addr, std::string& message)
{
	LOG_DEBUG << "received from : " << addr.toIpPort() << ", msg:" << message;
}

void defaultErrorCallback(uint64_t net_id, int32_t error_id)
{
	LOG_DEBUG << "Error. net id : " << net_id << ", err:" << error_id;
}

//返回智能指针,不需要释放内存
std::shared_ptr<INetWrok> getNetwork(NetMode mode)
{
	return std::shared_ptr<INetWrok>(new Network(mode));
}


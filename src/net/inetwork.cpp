#include "inetwork.h"
#include "network/network.h"
#include "../common/log/log.h"

void defaultAcceptCallback(uint64_t net_id)
{
	LOG_DEBUG << "accepted socket : " << net_id;
}

void defaultConnectCallback(uint64_t net_id)
{
	LOG_DEBUG << "connected socket : " << net_id;
}

void defaultDisConnectCallback(uint64_t net_id)
{
	LOG_DEBUG << "disconnected socket : " << net_id;
}

void defaultReceiveCallback(const uint64_t net_id, Buffer* buf, size_t)
{
	LOG_DEBUG << "received socket : " << net_id;
	
	buf->dropAll();
}

void defaultErrorCallback(uint64_t net_id, int32_t error_id)
{
	LOG_DEBUG << "error socket : " << net_id;
}

/*
INetWrok* getNetwork(NetMode mode)
{
	return new Network(mode);
}

void rleaseNetwork(INetWrok** network)
{
	(*network)->rlease();
	delete* network;
	*network = nullptr;
}
*/

//��������ָ��,����Ҫ�ͷ��ڴ�
std::shared_ptr<INetWrok> getNetwork(NetMode mode)
{
	return std::shared_ptr<INetWrok>(new Network(mode));
}


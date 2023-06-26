#include "poll.h"

#ifndef _WIN32
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

bool createPoll(Network* network)
{
	m_network = network;
	
	m_events.resize(16);

	m_epollFd = ::epoll_create1(0);

	return m_epollFd != -1;
}

void destoryPoll()
{
	for (auto& thread : m_threads)
	{
		if (thread.joinable())
			thread.join();
	}

	::close(m_epollFd);

	m_network->NETWARN << "[IOCP] destoryPoll.";
}

bool addPoll(std::shared_ptr<BaseNetObj> netObj)
{
	if (netObj == nullptr) return false;

}

void delPoll(std::shared_ptr<BaseNetObj> netObj)
{


}

bool enableReadPoll(std::shared_ptr<BaseNetObj> netObj, bool enable)
{


}

bool enableWritePoll(std::shared_ptr<BaseNetObj> netObj, bool enable)
{


}

int32_t waitPoll()
{


}

#endif
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

	m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);

	if (m_epollFd < 0)
	{
		m_network->NETWARN << "[EPOLL] create EPOLL FD fail.";

		return false;
	}

	//创建work线程
}

void destoryPoll()
{
	for (auto& thread : m_threads)
	{
		if (thread.joinable())
			thread.join();
	}

	::close(m_epollFd);

	m_network->NETWARN << "[EPOLL] destoryPoll.";
}

bool addPoll(std::shared_ptr<BaseNetObj> netObj)
{
	if (netObj == nullptr) return false;

	epoll_event et;
	et.data.u64 = netObj->getNetID();

	if (netObj->getNetStatus() == Connecting)
	{
		et.events = EPOLLOUT;

		
	}

	if (netObj->getNetMode() == (int)NetMode::TCP)
	{
		//只有TCP需要Listen
		if (netObj->isListen())
		{
			
		}
	}
	else if (netObj->getNetMode() == (int)NetMode::UDP)
	{
		
	}
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
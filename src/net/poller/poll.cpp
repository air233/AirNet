#include "poll.h"
#include "../network/network.h"
#include <thread>


void Poll::workPoll()
{

}

//主线程调用
void Poll::processPoll()
{
	std::queue<NetJob> net_jobs;
	{
		std::lock_guard<std::mutex> mylockguard(m_mutex);
		net_jobs.swap(m_net_jobs);
	}
	
	while (!net_jobs.empty())
	{
		NetJob job = net_jobs.front();
		net_jobs.pop();
		
		auto netObj = m_network->getNetObj2(job.m_net_id);

		if (netObj == nullptr)
		{
			m_network->NETDEBUG << "net job not find:" << job.m_net_id;
			continue;
		}

		switch (job.m_type)
		{
		case JobAccept:
			m_network->m_onAccept(job.m_net_id);
			break;

		case JobConnect:
			/*netid,连接错误码*/
			m_network->m_onConnect(job.m_net_id, netObj->getError());
			break;

		case JobDisconnect:
			m_network->m_onDisconnect(job.m_net_id);
			m_network->deleteNetObj(job.m_net_id);
			break;

		case JobReveive:
			m_network->m_onRecv(job.m_net_id, netObj->inputBuffer());
			break;

		case JobError:
			m_network->m_onError(job.m_net_id, netObj->getError());
			m_network->deleteNetObj(job.m_net_id);
			break;

		default:
			m_network->NETDEBUG << "net job not type. id:" << job.m_net_id<<", type:"<< job.m_type;
			break;
		}


	}

}

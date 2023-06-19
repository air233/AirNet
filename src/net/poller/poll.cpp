#include "poll.h"
#include "../network/network.h"
#include <thread>

NetJob::NetJob()
{
	m_type = JobNone;
	m_net_id = 0;
	m_error = 0;

}

NetJob::NetJob(const NetJob& job)
{
	m_type = job.m_type;
	m_net_id = job.m_net_id;
	m_error = job.m_error;
}

//(主线程调用）
void Poll::processJob()
{
	std::queue<std::shared_ptr<NetJob>> net_jobs;
	{
		std::lock_guard<std::mutex> lockguard(m_mutex);
		net_jobs.swap(m_net_jobs);
	}
	
	while (!net_jobs.empty())
	{
		std::shared_ptr<NetJob> job = net_jobs.front();
		net_jobs.pop();
		
		if (job == nullptr) continue;

		auto netObj = m_network->getNetObj2(job->m_net_id);

		if (netObj == nullptr)
		{
			m_network->NETDEBUG << "net job not find:" << job->m_net_id;
			continue;
		}

		switch (job->m_type)
		{
		case JobAccept:
			m_network->m_onAccept(job->m_net_id);
			break;

		case JobConnect:
			/*netid,连接错误码 0为成功*/
			m_network->m_onConnect(job->m_net_id, job->m_error);

			if (job->m_error != NET_SUCCESS)
			{
				m_network->deleteNetObj(job->m_net_id);
			}
			break;

		case JobDisconnect:
			m_network->m_onDisconnect(job->m_net_id);
			m_network->deleteNetObj(job->m_net_id);
			break;

		case JobReveive:
			m_network->m_onRecv(job->m_net_id, netObj->outputBuffer());
			break;

		case JobError:
			m_network->m_onError(job->m_net_id, job->m_error);
			m_network->deleteNetObj(job->m_net_id);
			break;

		default:
			m_network->NETDEBUG << "net job not type. net id:" << job->m_net_id<<", type:"<< job->m_type;
			break;
		}
	}
}

void Poll::PostAccpetJob(std::shared_ptr<BaseNetObj> netObj)
{
	netObj->setNetStatus(Connected);

	std::shared_ptr<NetJob> job = std::make_shared<NetJob>();
	job->m_type = JobAccept;
	job->m_net_id = netObj->getNetID();
	pushJob(job);
}

void Poll::PostConnectJob(std::shared_ptr<BaseNetObj> netObj, int err)
{
	if (err == NET_SUCCESS)
	{
		netObj->setNetStatus(Connected);
	}
	m_network->asyncConnectResult(netObj->getNetID(), err);

	std::shared_ptr<NetJob> job = std::make_shared<NetJob>();
	job->m_type = JobConnect;
	job->m_net_id = netObj->getNetID();
	job->m_error = err;
	pushJob(job);
}

void Poll::PostErrorJob(std::shared_ptr<BaseNetObj> netObj, int err)
{
	std::shared_ptr<NetJob> job = std::make_shared<NetJob>();
	job->m_type = JobError;
	job->m_net_id = netObj->getNetID();
	job->m_error = err;
	pushJob(job);
}

void Poll::PostDisConnectJob(std::shared_ptr<BaseNetObj> netObj, int err)
{
	std::shared_ptr<NetJob> job = std::make_shared<NetJob>();
	job->m_type = JobDisconnect;
	job->m_net_id = netObj->getNetID();
	job->m_error = err;
	pushJob(job);
}

void Poll::PostRecvJob(std::shared_ptr<BaseNetObj> netObj, char* buff, int len)
{
	{
		//放入output buffer
		std::lock_guard<std::mutex> gurad(netObj->outputMutex());
		netObj->outputBuffer()->pushCString(buff, len);
	}

	std::shared_ptr<NetJob> job = std::make_shared<NetJob>();
	job->m_type = JobReveive;
	job->m_net_id = netObj->getNetID();
	job->m_error = 0;
	pushJob(job);
}

void Poll::pushJob(std::shared_ptr<NetJob> job)
{
	std::lock_guard<std::mutex> lockguard(m_mutex);
	m_net_jobs.push(job);
}

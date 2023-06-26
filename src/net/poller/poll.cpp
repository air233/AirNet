#include "poll.h"
#include "../network/network.h"
#include "../netobj/udpnetobj.h"
#include <thread>
#include <iostream>

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
			m_network->NETDEBUG << "net job not find:" << job->m_net_id << ",type:" << job->m_type << ",err:" << job->m_error;
			continue;
		}

		int ret = 0;

		if (job->m_type == JobNewConn)
		{
			m_network->m_onNewConnect(job->m_net_id);

			//开启监听读事件
			enableReadPoll(netObj, true);
		}
		else if (job->m_type == JobConnect)
		{
			/*netid,连接错误码 0为成功*/
			m_network->m_onConnect(job->m_net_id, job->m_error);

			if (job->m_error != NET_SUCCESS)
			{
				m_network->deleteNetObj(job->m_net_id);
			}
		}
		else if (job->m_type == JobDisconnect)
		{
			m_network->m_onDisconnect(job->m_net_id);

			m_network->deleteNetObj(job->m_net_id);
		}
		else if (job->m_type == JobReveive)
		{
			Buffer tempBuff;
			{
				/*取出buffer数据*/
				std::lock_guard<std::mutex> lock(netObj->outputMutex());
				tempBuff.append(*netObj->outputBuffer());
			}

			/*处理数据*/
			m_network->m_onRecv(job->m_net_id, &tempBuff);

			if(tempBuff.size() != 0)
			{
				/*将未使用数据放回*/
				std::lock_guard<std::mutex> lock(netObj->outputMutex());
				netObj->outputBuffer()->insert(tempBuff.begin(), tempBuff.size());
			}
		}
		else if (job->m_type == JobReveiveFrom)
		{
			Message msg;
			ret = netObj->getMessage(msg);

			if (ret)
			{
				m_network->m_onRecvFrom(msg.m_addr, msg.m_message);
			}
		}
		else if (job->m_type == JobError)
		{
			m_network->m_onError(job->m_net_id, job->m_error);

			if (m_network->getNetMode() == (int)NetMode::TCP)
			{
				m_network->deleteNetObj(job->m_net_id);
			}
		}
		else
		{
			m_network->NETDEBUG << "net job not type. net id:" << job->m_net_id<<", type:"<< job->m_type;
		}
	}
}

void Poll::PostNewConnectJob(std::shared_ptr<BaseNetObj> netObj)
{
	netObj->setNetStatus(Connected);

	std::shared_ptr<NetJob> job = std::make_shared<NetJob>();
	job->m_type = JobNewConn;
	job->m_net_id = netObj->getNetID();
	pushJob(job);
}

void Poll::PostConnectJob(std::shared_ptr<BaseNetObj> netObj, int err)
{
	if (err == NET_SUCCESS)
	{
		netObj->setNetStatus(Connected);
	}

	//通知结果
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

void Poll::PostRecvFromJob(std::shared_ptr<BaseNetObj> netObj, InetAddress& addr, char* buff, int len)
{
	auto udpNetObj = std::static_pointer_cast<UDPNetObj>(netObj);
	if (udpNetObj == nullptr) return;

	Message recvMsg;
	recvMsg.m_addr = addr;
	recvMsg.m_message.assign(buff, buff+len);
	udpNetObj->pushMessage(recvMsg);

	std::shared_ptr<NetJob> job = std::make_shared<NetJob>();
	job->m_type = JobReveiveFrom;
	job->m_net_id = netObj->getNetID();
	job->m_error = 0;
	pushJob(job);
}

void Poll::pushJob(std::shared_ptr<NetJob>& job)
{
	std::lock_guard<std::mutex> lockguard(m_mutex);
	m_net_jobs.push(job);
}

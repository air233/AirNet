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

//(���̵߳��ã�
void Poll::processJob()
{
	std::queue<std::shared_ptr<NetJob>> net_jobs;
	{
		std::lock_guard<std::mutex> lockguard(m_mutex);
		net_jobs.swap(m_net_jobs);
	}
	
	Message msg;

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

		switch (job->m_type)
		{
		case JobNewConn:
			m_network->m_onNewConnect(job->m_net_id);
			
			//�����������¼�
			enableReadPoll(netObj, true);
			break;

		case JobConnect:
			/*netid,���Ӵ����� 0Ϊ�ɹ�*/
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

		case JobReveiveFrom:
			ret = netObj->getMessage(msg);
			if (ret == true)
				m_network->m_onRecvFrom(msg.m_addr, msg.m_message);
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

	//֪ͨ���
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
		//����output buffer
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
	recvMsg.m_addr = InetAddress(addr);
	recvMsg.m_message.assign(buff, buff+len);
	udpNetObj->pushMessage(recvMsg);

	std::shared_ptr<NetJob> job = std::make_shared<NetJob>();
	job->m_type = JobReveiveFrom;
	job->m_net_id = netObj->getNetID();
	job->m_error = 0;
	pushJob(job);
}

void Poll::pushJob(std::shared_ptr<NetJob> job)
{
	std::lock_guard<std::mutex> lockguard(m_mutex);
	m_net_jobs.push(job);
}

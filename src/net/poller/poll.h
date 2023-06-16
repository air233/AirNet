#pragma once
#include "../nettype.h"
#include "../netobj/basenetobj.h"
#include <memory>
#include <queue>
#include <mutex>

#ifdef _WIN32
#include <Windows.h>
#else
#endif

/*
����ICOP��EPOLLģ�Ͳ�ͬ
����ʹ��Job���������н���
����Job��Ϣ�������������̴߳���
*/

struct NetJob
{
	JobType m_type = JobNone;
	uint64_t m_net_id = 0;
};

class Network;

class Poll
{
public:
	Poll():m_network(nullptr), m_listen(0),
#ifdef _WIN32
		m_completionPort(nullptr)
#else
		m_epollFd(0)
#endif
	{};

	~Poll() {};

	bool createPoll(Network* network);

	void destoryPoll();

	bool addPoll(std::shared_ptr<BaseNetObj> netObj);

	void delPoll(std::shared_ptr<BaseNetObj> netObj);

	int enablePoll(std::shared_ptr<BaseNetObj> netObj, bool read_enable, bool write_enable);

	int32_t waitPoll();

	void workPoll();

	//������
	void processPoll();

private:
	Network* m_network;
	SOCKET m_listen;

	/*�����б�*/
	std::mutex m_mutex;
	std::queue<NetJob> m_net_jobs;

#ifdef _WIN32
	HANDLE m_completionPort;
#else
	int m_epollFd;
#endif
};





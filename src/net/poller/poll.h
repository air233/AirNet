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
由于ICOP和EPOLL模型不同
这里使用Job队列来进行解耦
产生Job信息后放入队列由主线程处理
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

	//处理函数
	void processPoll();

private:
	Network* m_network;
	SOCKET m_listen;

	/*任务列表*/
	std::mutex m_mutex;
	std::queue<NetJob> m_net_jobs;

#ifdef _WIN32
	HANDLE m_completionPort;
#else
	int m_epollFd;
#endif
};





#pragma once
#include "../nettype.h"
#include "../netobj/basenetobj.h"

#include <memory>
#include <queue>
#include <mutex>
#include <map>
#include <thread>

#ifdef _WIN32
#include <Windows.h>

class IO_CONTEXT;
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
	int32_t m_error = 0;
};

class Network;

class Poll
{
public:
	Poll():m_network(nullptr), m_run(true),
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

	void pushJob(NetJob& job);

	//main处理函数
	void processPoll();

private:
	Network* m_network;
	bool m_run;
	
	/*任务列表*/
	std::mutex m_mutex;
	std::queue<NetJob> m_net_jobs;
	std::vector<std::thread> m_threads;


#ifdef _WIN32
	HANDLE m_completionPort;
	std::mutex m_iocontexts_mutex;
	std::map<uint64_t, IO_CONTEXT*> m_iocontexts;

	void PostAccept(std::shared_ptr<BaseNetObj> netObj);
	void WorkerThread();
	
#else
	int m_epollFd;
#endif
};





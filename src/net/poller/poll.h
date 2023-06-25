#pragma once
#include "../nettype.h"
#include "../netobj/basenetobj.h"
#include "../until/InetAddress.h"

#include <memory>
#include <queue>
#include <mutex>
#include <map>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#include <MSWSock.h>
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
	JobType m_type;
	uint64_t m_net_id;
	int32_t m_error;

	NetJob();
	NetJob(const NetJob& job);
};

class Network;

class Poll
{
public:
	Poll():m_network(nullptr), m_run(true),
#ifdef _WIN32
		m_completionPort(nullptr),
		m_ConnectEx(nullptr)
#else
		m_epollFd(0)
#endif
	{};

	~Poll() {};

	bool createPoll(Network* network);
	void destoryPoll();

	bool addPoll(std::shared_ptr<BaseNetObj> netObj);
	void delPoll(std::shared_ptr<BaseNetObj> netObj);

	bool enableReadPoll(std::shared_ptr<BaseNetObj> netObj, bool enable);
	bool enableWritePoll(std::shared_ptr<BaseNetObj> netObj, bool enable);

	int32_t waitPoll();
	void workPoll();

	void pushJob(std::shared_ptr<NetJob>& job);
	//主线程处理函数
	void processJob();
private:
	Network* m_network;
	bool m_run;
	
	/*任务列表*/
	std::mutex m_mutex;
	std::queue<std::shared_ptr<NetJob>> m_net_jobs;

	std::vector<std::thread> m_threads;

	void PostNewConnectJob(std::shared_ptr<BaseNetObj> netObj);
	void PostConnectJob(std::shared_ptr<BaseNetObj> netObj, int err);
	void PostErrorJob(std::shared_ptr<BaseNetObj> netObj, int err);
	void PostDisConnectJob(std::shared_ptr<BaseNetObj> netObj, int err);
	void PostRecvJob(std::shared_ptr<BaseNetObj> netObj,char* buff,int len);
	void PostRecvFromJob(std::shared_ptr<BaseNetObj> netObj,InetAddress& addr, char* buff, int len);
#ifdef _WIN32
	HANDLE m_completionPort;
	LPFN_CONNECTEX m_ConnectEx;
	
	void WorkerThread();

	bool LoadConnectEx();
	void PostAcceptEvent(std::shared_ptr<BaseNetObj> netObj);
	bool PostConnectEvent(std::shared_ptr<BaseNetObj> netObj);
	bool PostRecvEvent(std::shared_ptr<BaseNetObj> netObj);
	bool PostSendEvent(std::shared_ptr<BaseNetObj> netObj);

	bool PostRecv(std::shared_ptr<BaseNetObj> netObj);
	bool PostRecvFrom(std::shared_ptr<BaseNetObj> netObj);
	bool PostSend(std::shared_ptr<BaseNetObj> netObj);
	bool PostSendTo(std::shared_ptr<BaseNetObj> netObj);
#else
	int m_epollFd;
#endif
};





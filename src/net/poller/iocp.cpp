#include "poll.h"

#ifdef _WIN32
#include "../network/network.h"
#include "../../common/buffer/buffer.h"
#include <functional>
#include <iostream>

constexpr int MAX_BUFFER_SIZE = 1024;

enum IOCompletionKey
{
	IOQuit   = 0,
	IOClinet = 1,
	IOListen = 2,
};

struct PER_IO_CONTEXT 
{
	OVERLAPPED Overlapped;
	SOCKET Socket;
	uint64_t NetID;
	WSABUF DataBuf;
	char Buffer[MAX_BUFFER_SIZE];
};

void Poll::WorkerThread()
{
	DWORD bytesTransferred;
	ULONG_PTR completionKey;
	LPOVERLAPPED overlapped;
	while (true) 
	{
		if (GetQueuedCompletionStatus(m_completionPort, &bytesTransferred, &completionKey, &overlapped, INFINITE) == FALSE) 
		{
			m_network->NETERROR << "Failed to get completion status. Error: " << GetLastError() << std::endl;
			continue;
		}
		std::cout << "[IOCP] WorkerThread2." << std::endl;;

		m_network->NETDEBUG << "[IOCP] WorkerThread2.";

		PER_IO_CONTEXT* ioContext = reinterpret_cast<PER_IO_CONTEXT*>(overlapped);
		auto netObj = m_network->getNetObj2(ioContext->NetID);
		if (netObj == nullptr)
		{
			m_network->NETERROR << "not find net obj." << ioContext->NetID;
			continue;
		}

		if (bytesTransferred == 0) 
		{
			// 客户端关闭连接
			// TODO:关闭事件
			continue;
		}

		if (completionKey == IOListen)
		{
			//TODO:linsten fd 可读
			if (m_network->getNetMode() == TCP)
			{
				//TODO:Accept函数
				m_network->NETDEBUG << "[IOCP] tcp accept" << ioContext->NetID;
			}
			else if (m_network->getNetMode() == UDP)
			{

			}
			else if (m_network->getNetMode() == KCP)
			{

			}
		}
		else
		{
			//TODO:client fd 可读

			// 处理收到的数据
			m_network->NETDEBUG << "Received: " << ioContext->Buffer << std::endl;
		
			{
				//放入output buffer
				std::lock_guard<std::mutex> gurad(netObj->outputMutex());
				netObj->outputBuffer()->pushCString(ioContext->Buffer, bytesTransferred);
			}

			//TODO:recv事件

		}
		memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
	}
}

bool Poll::createPoll(Network* network)
{
	m_network = network;

	m_listen = m_network->getListenSock();

	//创建完成IO端口
	m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_completionPort == NULL)
	{
		m_network->NETERROR << "[IOCP] CreateIoCompletionPort fail.";
		return false;
	}

	//获取线程数
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	DWORD numThreads = systemInfo.dwNumberOfProcessors;

	//创建工作线程
	for (int i = 0; i < numThreads; ++i) 
	{
		m_threads.emplace_back(&Poll::WorkerThread,this);
	}

	m_network->NETDEBUG << "[IOCP] createIoCompletionPort succ. work count:" << numThreads;
	return true;
}

bool Poll::addPoll(std::shared_ptr<BaseNetObj> netObj)
{
	PER_IO_CONTEXT* ioContext = new PER_IO_CONTEXT();
	ioContext->Socket = netObj->fd();
	ioContext->NetID = netObj->getNetID();
	memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
	ioContext->DataBuf.buf = ioContext->Buffer;
	ioContext->DataBuf.len = sizeof(ioContext->Buffer);

	int32_t IOCompletionKey = IOClinet;
	if (netObj->getNetMode() == TCP && netObj->fd() == m_listen)
	{
		IOCompletionKey = IOListen;
	}

	//将netObj与完成IO端口绑定
	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(netObj->fd()), m_completionPort, IOCompletionKey, 0) == nullptr) 
	{
		m_network->NETERROR << "[IOCP] Failed to associate client socket with IOCP. Error: " << GetLastError() << std::endl;
		delete ioContext;
		return false;
	}

	{
		std::lock_guard<std::mutex> guard(m_iocontexts_mutex);
		m_iocontexts.insert(std::make_pair(netObj->fd(), ioContext));
	}

	m_network->NETDEBUG << "[IOCP] addPoll. net id: " << netObj->getNetID();
	return true;
}


void Poll::delPoll(std::shared_ptr<BaseNetObj> netObj)
{
	std::lock_guard<std::mutex> guard(m_iocontexts_mutex);
	auto iter = m_iocontexts.find(netObj->getNetID());
	if (iter == m_iocontexts.end()) return;

	delete iter->second;
	m_iocontexts.erase(netObj->getNetID());
}

int32_t Poll::waitPoll()
{
	return 1;
}

void Poll::workPoll()
{

}

int Poll::enablePoll(std::shared_ptr<BaseNetObj> netObj, bool read_enable, bool write_enable)
{
	return 0;
}

void Poll::destoryPoll()
{
	for (auto& thread : m_threads)
	{
		PostQueuedCompletionStatus(m_completionPort, 0, IOQuit, nullptr);
	}

	for (auto& thread : m_threads)
	{
		
		thread.join();
	}

	CloseHandle(m_completionPort);
	m_network->NETDEBUG << "[IOCP] destoryPoll.";
}


#endif
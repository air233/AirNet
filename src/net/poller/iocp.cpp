#include "poll.h"

#ifdef _WIN32
constexpr static size_t MaxBufferSize = 1024 * 1;
constexpr static size_t NumberOfThreads = 1;

HANDLE hIOCP = INVALID_HANDLE_VALUE;
SOCKET serverSocket = INVALID_SOCKET;
std::vector<std::thread> threadGroup;
std::atomic_bool isShutdown{ false };

enum class IOType 
{
	IORead,
	IOWrite
};

struct IOContext 
{
	OVERLAPPED overlapped {};
	WSABUF wsaBuf { MaxBufferSize, buffer };
	CHAR buffer[MaxBufferSize]{};
	IOType type{};
	SOCKET socket = INVALID_SOCKET;
	DWORD nBytes = 0;
};

bool Poll::createPoll(Network* network)
{
	m_network = network;

	//m_completionPort = NULL;

	//m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	//return (m_completionPort != NULL);
	return true;
}

bool Poll::addPoll(std::shared_ptr<BaseNetObj> netObj)
{
	//uint64_t sockcontext_completionkey = 0;

	//HANDLE handle = CreateIoCompletionPort((HANDLE)netObj->fd(), m_completionPort, netObj->getNetID(), sockcontext_completionkey);

	//return handle != NULL;
	return true;
}

void Poll::delPoll(std::shared_ptr<BaseNetObj> netObj)
{
	//HANDLE removedPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, m_completionPort, netObj->getNetID(), 0);
	
	//return removedPort != NULL;
}

int32_t Poll::waitPoll()
{

	return 1;


}

int Poll::enablePoll(std::shared_ptr<BaseNetObj> netObj, bool read_enable, bool write_enable)
{

	return 1;


}

void Poll::destoryPoll()
{

}


#endif
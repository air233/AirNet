#include "poll.h"

#ifdef _WIN32
#include "../network/network.h"
#include "../socket/socketops.h"
#include "../until/InetAddress.h"
#include "../../common/buffer/buffer.h"
#include <functional>
#include <iostream>

const int MAX_ACCPET_SIZE = 511;
constexpr int MAX_BUFFER_SIZE = 1024;

enum class IOType : int
{
	IONone,
	IOAccept,
	IORecv,
	IOSend,
	IOConn,
};

char* CIOType[5] =
{
	"None",
	"Accept",
	"Recv",
	"Send",
	"Conn",
};

struct IO_CONTEXT
{
	OVERLAPPED Overlapped;
	SOCKET Socket;
	uint64_t NetID;
	WSABUF DataBuf;
	char Buffer[MAX_BUFFER_SIZE];
	IOType type;
};

bool Poll::LoadConnectEx()
{
	SOCKET sock;
	DWORD dwBytes;
	int rc;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return FALSE;

	{
		GUID guid = WSAID_CONNECTEX;
		rc = WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guid, sizeof(guid),
			&m_ConnectEx, sizeof(m_ConnectEx),
			&dwBytes, NULL, NULL);
		if (rc != 0)	
			return FALSE;
	}

	closesocket(sock);

	return TRUE;
}

void Poll::PostAcceptEvent(std::shared_ptr<BaseNetObj> listenNetObj)
{
	SOCKET clientSock = createTCPSocket(listenNetObj->localAddress().family());
	if (clientSock == INVALID_SOCKET)
	{
		m_network->NETERROR << "PostAccept Failed to createTCPSocket. Error: " << GetLastError() << std::endl;
		return;
	}

	IO_CONTEXT* ioContext = new IO_CONTEXT();
	ioContext->NetID = 0;
	ioContext->Socket = clientSock;
	memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
	ioContext->DataBuf.buf = ioContext->Buffer;
	ioContext->DataBuf.len = sizeof(ioContext->Buffer);
	ioContext->type = IOType::IOAccept;

	DWORD dwByteRcv = 0;
	if(false == AcceptEx(listenNetObj->fd(), clientSock, &ioContext->Buffer,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&dwByteRcv,
		(LPOVERLAPPED)ioContext))
	{
		if (GetLastError() != WSA_IO_PENDING)
		{
			m_network->NETERROR << "PostAccept AcceptEx Failed. Error: " << GetLastError();
		}
	}
}

bool Poll::PostConnectEvent(std::shared_ptr<BaseNetObj> netObj)
{
	IO_CONTEXT* ioContext = new IO_CONTEXT();
	ioContext->NetID = netObj->getNetID();
	ioContext->Socket = netObj->fd();
	memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
	ioContext->DataBuf.buf = ioContext->Buffer;
	ioContext->DataBuf.len = sizeof(ioContext->Buffer);
	ioContext->type = IOType::IOConn;

	// 绑定本地地址和端口
	sockaddr_storage localAddr;
	memset(&localAddr, 0, sizeof localAddr);
	localAddr.ss_family = netObj->peerAddress().family();
	if (::bind(netObj->fd(), reinterpret_cast<sockaddr*>(&localAddr), sizeof(localAddr)) == SOCKET_ERROR)
	{
		int err = GetLastError();
		m_network->NETERROR << "[IOCP] bind address fail: " << err;
		PostConnectJob(netObj, err);
		return false;
	}

	auto peeraddr = netObj->peerAddress().getSockAddr();
	DWORD dwByteRcv = 0;
	if(false == m_ConnectEx(netObj->fd(), (sockaddr*)peeraddr, sizeof(*peeraddr), NULL, 0, NULL, (LPOVERLAPPED)ioContext))
	{
		int err = GetLastError();
		if (err != WSA_IO_PENDING)
		{
			m_network->NETERROR << "PostConnect Connect Failed. Error: " << err;
			PostConnectJob(netObj, err);
			return false;
		}
	}

	return true;
}

bool Poll::PostRecvEvent(std::shared_ptr<BaseNetObj> netObj)
{
	if (netObj->getNetMode() == (int)NetMode::TCP)
	{
		return PostRecv(netObj);
	}
}

bool Poll::PostRecv(std::shared_ptr<BaseNetObj> netObj)
{
	IO_CONTEXT* ioContext = new IO_CONTEXT();
	ioContext->NetID = netObj->getNetID();
	ioContext->Socket = netObj->fd();
	memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
	ioContext->DataBuf.buf = ioContext->Buffer;
	ioContext->DataBuf.len = sizeof(ioContext->Buffer);
	ioContext->type = IOType::IORecv;

	DWORD flags = 0;
	DWORD bytesReceived;

	int ret = WSARecv(netObj->fd(), &ioContext->DataBuf, 1, &bytesReceived, &flags, (LPOVERLAPPED)ioContext, 0);
	if (ret != 0)
	{
		int32_t err = GetLastError();

		if (err != WSA_IO_PENDING)
		{
			PostErrorJob(netObj, err);
			return false;
		}
		m_network->NETDEBUG << "[IOCP] PostRecv.";
		return true;
	}
}

bool Poll::PostSendEvent(std::shared_ptr<BaseNetObj> netObj)
{
	if (netObj->getNetMode() == (int)NetMode::TCP)
	{
		return PostSend(netObj);
	}
}

bool Poll::PostSend(std::shared_ptr<BaseNetObj> netObj)
{
	std::string sendData;
	{
		std::lock_guard<std::mutex> gurad(netObj->inputMutex());
		if (netObj->inputBuffer()->size() == 0)
		{
			//std::cout << "[IOCP] Send over." << std::endl;
			return true;
		}
		else
		{
			netObj->inputBuffer()->peekString(sendData, MAX_BUFFER_SIZE);
		}
	}

	IO_CONTEXT* ioContext = new IO_CONTEXT();
	ioContext->NetID = netObj->getNetID();
	ioContext->Socket = netObj->fd();
	memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
	memcpy(ioContext->Buffer, sendData.c_str(), sendData.size());
	ioContext->DataBuf.buf = ioContext->Buffer;
	ioContext->DataBuf.len = sizeof(ioContext->Buffer);
	ioContext->type = IOType::IOSend;

	DWORD lpNumberOfBytesSent;
	int ret = WSASend(netObj->fd(), &ioContext->DataBuf, 1, &lpNumberOfBytesSent, 0, (LPOVERLAPPED)ioContext, 0);
	if (ret != 0)
	{
		int32_t err = GetLastError();

		if (err != WSA_IO_PENDING)
		{
			PostErrorJob(netObj, err);
			return false;
		}

		//std::cout << "[IOCP] PostSend." << std::endl;
		m_network->NETDEBUG << "[IOCP] PostSend.";
		return true;
	}
}


void Poll::WorkerThread()
{
	DWORD bytesTransferred;
	ULONG_PTR completionKey;
	LPOVERLAPPED overlapped;

	while (true) 
	{
		BOOL result = GetQueuedCompletionStatus(m_completionPort, &bytesTransferred, &completionKey, &overlapped, INFINITE);
		
		if (overlapped == NULL)
		{
			m_network->NETWARN << "[IOCP] work thread quit.";
			break;
		}

		IO_CONTEXT* ioContext = reinterpret_cast<IO_CONTEXT*>(overlapped);

		if (result == FALSE)
		{
			int32_t error = GetLastError();
			m_network->NETWARN <<"Net id:" << ioContext->NetID << ": Failed to get completion status. Error: " << error;

			auto netObj = m_network->getNetObj2(ioContext->NetID);
			if (netObj == nullptr)
			{
				m_network->NETWARN << "[IOCP] recv not find net obj." << ioContext->NetID;
				delete ioContext;
				continue;
			}

			if (error == WAIT_TIMEOUT || error == ERROR_NETNAME_DELETED)
			{
				PostDisConnectJob(netObj, error);
			}
			else
			{
				PostErrorJob(netObj, error);
			}

			delete ioContext;
			continue;
		}

		//std::cout << "[IOCP] WorkerThread1 process event:" << CIOType[(int)ioContext->type] << std::endl;
		m_network->NETDEBUG << "[IOCP] WorkerThread1 process event:" << CIOType[(int)ioContext->type];

		if (ioContext->type == IOType::IOAccept)
		{
			if (m_network->getNetMode() == (int)NetMode::TCP)
			{
				//继续投递Accpet
				PostAcceptEvent(m_network->getServerNetObj());
				m_network->NETDEBUG << "[IOCP] TCP accept" << ioContext->NetID;
			
				//================处理新连接客户端================
				//clientSock关联listenSock
				SOCKET listenSocket = m_network->getServerNetObj()->fd();
				setsockopt(ioContext->Socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenSocket, sizeof SOCKET);
				
				//生成一个client NetObj
				auto clientNetObj = m_network->makeNetObj(m_network, ioContext->Socket);
				if (false == m_network->insertNetObj(clientNetObj))
				{
					m_network->NETERROR << "Failed to insert netObj. net id: " << clientNetObj->getNetID();
					closeSocket(ioContext->Socket);
					delete ioContext;
					continue;
				}
				
				//绑定地址信息
				sockaddr_storage addr;
				if (0 == getLocalAddr(ioContext->Socket, addr))
				{
					 clientNetObj->setlocalAddress(InetAddress(addr));
					 //m_network->NETDEBUG << "[IOCP] new client local addr:" << localAddr.toIpPort();
				}
				if (0 == getPeerAddr(ioContext->Socket, addr))
				{
					clientNetObj->setpeerAddress(InetAddress(addr));
					//m_network->NETDEBUG << "[IOCP] new client peer addr:" << peerAddr.toIpPort();
				}

				//投递一个AccpetJob
				PostAccpetJob(clientNetObj);
			}
		}
		else if (ioContext->type == IOType::IORecv)
		{
			auto netObj = m_network->getNetObj2(ioContext->NetID);
			if (netObj == nullptr)
			{
				m_network->NETERROR << "[IOCP] recv not find net obj." << ioContext->NetID;
				delete ioContext;
				continue;
			}

			if (bytesTransferred == 0)
			{
				// 客户端关闭连接
				m_network->NETDEBUG << "[IOCP] connections disconnect.";
				//std::cout << "[IOCP] connections disconnect:" << GetLastError() << std::endl;
				PostDisConnectJob(netObj, GetLastError());
				delete ioContext;
				continue;
			}

			//处理收到的数据
			PostRecvJob(netObj, ioContext->Buffer, bytesTransferred);

			//继续接收事件
			PostRecvEvent(netObj);
		}
		else if (ioContext->type == IOType::IOConn)
		{
			auto netObj = m_network->getNetObj2(ioContext->NetID);
			if (netObj == nullptr)
			{
				m_network->NETERROR << "[IOCP] connect not find net obj." << ioContext->NetID;
				delete ioContext;
				continue;
			}

			int updateContext = 1;
			if (setsockopt(ioContext->Socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, reinterpret_cast<const char*>(&updateContext), sizeof(updateContext)) == SOCKET_ERROR)
			{
				m_network->NETDEBUG << "[IOCP] update connect context fail: " << GetLastError();
				PostErrorJob(netObj, GetLastError());
				delete ioContext;
				continue;
			}

			//更新地址信息
			sockaddr_storage addr;
			if (0 == getLocalAddr(ioContext->Socket, addr))
			{
				netObj->setlocalAddress(InetAddress(addr));
			}

			if (0 == getPeerAddr(ioContext->Socket, addr))
			{
				netObj->setpeerAddress(InetAddress(addr));
			}
			
			//生成一个连接Job
			PostConnectJob(netObj, NET_SUCCESS);
			m_network->NETDEBUG << "[IOCP] async connect success. net id:" << ioContext->NetID;
		}
		else if (ioContext->type == IOType::IOSend)
		{
			std::cout << "send" << std::endl;

			auto netObj = m_network->getNetObj2(ioContext->NetID);
			if (netObj == nullptr)
			{
				m_network->NETERROR << "[IOCP] send not find net obj." << ioContext->NetID;
				delete ioContext;
				continue;
			}

			PostSendEvent(netObj);
		}

		delete ioContext;
	}
}

bool Poll::createPoll(Network* network)
{
	m_network = network;

	if (LoadConnectEx() == false)
	{
		m_network->NETERROR << "[IOCP] LoadConnectEx fail.";
	}

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

	m_network->NETDEBUG << "[IOCP] createIoCompletionPort succ. work thread count:" << numThreads;
	return true;
}

bool Poll::addPoll(std::shared_ptr<BaseNetObj> netObj)
{
	//绑定完成端口
	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(netObj->fd()), m_completionPort, 0, 0) == nullptr)
	{
		m_network->NETERROR << "[IOCP] failed to associate client socket with IOCP. Error: " << GetLastError() << std::endl;
		return false;
	}

	m_network->NETDEBUG << "[IOCP] addPoll bind IoCompletionPort. net id: " << netObj->getNetID() << ", status:" << netObj->getNetStatus();

	//只有TCP需要Listen
	if (netObj->getNetMode() == (int)NetMode::TCP && netObj->isListen())
	{
		//预先生成 投递AccpetEx事件
		for (int i = 0; i < MAX_ACCPET_SIZE; i++)
		{
			PostAcceptEvent(netObj);
		}

		m_network->NETDEBUG << "[IOCP] PostAcceptEvent.";
		return true;
	}

	//处理异步连接
	if (netObj->getNetStatus() == Connecting)
	{
		m_network->NETDEBUG << "[IOCP] PostConnectEvent.";
		return PostConnectEvent(netObj);
	}

	return true;
}

void Poll::delPoll(std::shared_ptr<BaseNetObj> netObj)
{
	(void)netObj;
}

int32_t Poll::waitPoll()
{
	return 1;
}

bool Poll::enablePoll(std::shared_ptr<BaseNetObj> netObj, bool read_enable, bool write_enable)
{
	if (netObj == nullptr)
	{
		return false;
	}

	bool ret = false;

	if (read_enable)
	{
		if (netObj->getNetMode() == (int)NetMode::TCP)
		{
			ret = PostRecvEvent(netObj);
		}

		//TODO:TCP|UDP
		if (ret == false)
		{
			return false;
		}
	}

	if (write_enable)
	{
		if (netObj->getNetMode() == (int)NetMode::TCP)
		{
			ret = PostSendEvent(netObj);
		}

		//TODO:TCP|UDP

		if (ret == false)
		{
			return false;
		}
	}

	return true;
}

void Poll::destoryPoll()
{
	for (auto& thread : m_threads)
	{
		Sleep(10);
		if (false == PostQueuedCompletionStatus(m_completionPort, 0, 0, nullptr))
		{
			m_network->NETWARN << "[IOCP] PostQueuedCompletionStatus error:" << GetLastError();
		}
	}

	for (auto& thread : m_threads)
	{
		if(thread.joinable())
			thread.join();
	}

	CloseHandle(m_completionPort);
	m_network->NETWARN << "[IOCP] destoryPoll.";
}

#endif
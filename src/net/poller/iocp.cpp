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

void Poll::PostAccept(std::shared_ptr<BaseNetObj> listenNetObj)
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

void Poll::PostConnect(std::shared_ptr<BaseNetObj> netObj)
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
		return;
	}

	auto peeraddr = netObj->peerAddress().getSockAddr();
	DWORD dwByteRcv = 0;
	if(false == m_ConnectEx(netObj->fd(), 
		(sockaddr*)peeraddr, 
		sizeof(*peeraddr), 
		NULL, 
		0, 
		NULL, 
		(LPOVERLAPPED)ioContext))
	{
		if (GetLastError() != WSA_IO_PENDING)
		{
			int err = GetLastError();
			m_network->NETERROR << "PostConnect Connect Failed. Error: " << err;
			PostConnectJob(netObj, err);
		}
	}
}

void Poll::PostRecv(std::shared_ptr<BaseNetObj> netObj)
{

}

void Poll::PostSend(std::shared_ptr<BaseNetObj> netObj)
{

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
			m_network->NETDEBUG << "[IOCP] work thread quit.";
			break;
		}
		IO_CONTEXT* ioContext = reinterpret_cast<IO_CONTEXT*>(overlapped);

		if (result == FALSE)
		{
			int32_t error = GetLastError();

			m_network->NETERROR <<"Net id:" << ioContext->NetID << ": Failed to get completion status. Error: " << error;

			if (error == WAIT_TIMEOUT || error == ERROR_NETNAME_DELETED)
			{
				auto netObj = m_network->getNetObj2(ioContext->NetID);

				if (netObj == nullptr)
				{
					m_network->NETERROR << "[IOCP] recv not find net obj." << ioContext->NetID;
					delete ioContext;
					continue;
				}

				//产生一个DisconnectJob
				PostDisConnectJob(netObj, error);
			}

			delete ioContext;
			continue;
		}

		m_network->NETDEBUG << "[IOCP] WorkerThread1 process event:" << CIOType[(int)ioContext->type];

		if (ioContext->type == IOType::IOAccept)
		{
			if (m_network->getNetMode() == (int)NetMode::TCP)
			{
				m_network->NETDEBUG << "[IOCP] TCP accept" << ioContext->NetID;

				//继续投递一个Accpet
				PostAccept(m_network->getServerNetObj());

				//clientSock关联listenSock
				SOCKET listenSocket = m_network->getServerNetObj()->fd();
				setsockopt(ioContext->Socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenSocket, sizeof SOCKET);
				
				//生成一个NetObj
				auto clientNetObj = m_network->makeNetObj(m_network, ioContext->Socket);
				if (false == m_network->insertNetObj(clientNetObj))
				{
					m_network->NETERROR << "Failed to insert netObj. net id: " << clientNetObj->getNetID();
					closeSocket(ioContext->Socket);
					continue;
				}
				
				//设置地址信息
				sockaddr_storage addr;
				if (0 == getLocalAddr(ioContext->Socket, addr))
				{
					 InetAddress localAddr(addr);
					 clientNetObj->setlocalAddress(localAddr);
					 m_network->NETDEBUG << "[IOCP] new client local addr:" << localAddr.toIpPort();
				}
				if (0 == getPeerAddr(ioContext->Socket, addr))
				{
					InetAddress peerAddr(addr);
					clientNetObj->setpeerAddress(peerAddr);
					m_network->NETDEBUG << "[IOCP] new client peer addr:" << peerAddr.toIpPort();
				}

				//将ClientSock与完成端口进行关联
				CreateIoCompletionPort((HANDLE)ioContext->Socket, m_completionPort, 0, 0);

				//投递一个接收事件
				ioContext->type = IOType::IORecv;
				ioContext->NetID = clientNetObj->getNetID();
				memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
				ioContext->DataBuf.buf = ioContext->Buffer;
				ioContext->DataBuf.len = sizeof(ioContext->Buffer);
				
				DWORD dwRecv, dwFlag = 0;
				auto ret = WSARecv(ioContext->Socket, &ioContext->DataBuf, 1, &dwRecv, &dwFlag, &(ioContext->Overlapped), 0);
				if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
				{
					m_network->NETDEBUG << "[IOCP] client WSARecv fail. error:" << GetLastError();
					PostErrorJob(clientNetObj, GetLastError());

					delete ioContext;
				}
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
				PostDisConnectJob(netObj, GetLastError());
				delete ioContext;
				continue;
			}
			//m_network->NETDEBUG << "[IOCP] recv size :" << bytesTransferred;
			
			//处理收到的数据
			PostRecvJob(netObj, ioContext->Buffer, bytesTransferred);

			//继续接收事件
			memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
			
			DWORD dwRecv, dwFlag = 0;
			auto ret = WSARecv(ioContext->Socket, &ioContext->DataBuf, 1, &dwRecv, &dwFlag, &(ioContext->Overlapped), 0);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				m_network->NETDEBUG << "[IOCP] client WSARecv fail. error:" << GetLastError();
				delete ioContext;

				PostErrorJob(netObj, GetLastError());
			}
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
				return;
			}

			//更新地址信息
			sockaddr_storage addr;
			getLocalAddr(ioContext->Socket, addr);
			netObj->setlocalAddress(InetAddress(addr));

			getPeerAddr(ioContext->Socket, addr);
			netObj->setpeerAddress(InetAddress(addr));

			m_network->NETDEBUG << "[IOCP] async connect success. net id:" << ioContext->NetID;
			//通知结果
			PostConnectJob(netObj, NET_SUCCESS);
			delete ioContext;
		}
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
			PostAccept(netObj);
		}
		
		m_network->NETDEBUG << "[IOCP] PostAccept.";
		return true;
	}

	//处理异步连接
	if (netObj->getNetStatus() == Connecting)
	{
		PostConnect(netObj);

		m_network->NETDEBUG << "[IOCP] PostConnect.";
		return true;
	}
	
	//TODO:其他事件


	//IO_CONTEXT* ioContext = new IO_CONTEXT();
	//ioContext->Socket = netObj->fd();
	//ioContext->NetID = netObj->getNetID();
	//memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
	//ioContext->DataBuf.buf = ioContext->Buffer;
	//ioContext->DataBuf.len = sizeof(ioContext->Buffer);
	
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

int Poll::enablePoll(std::shared_ptr<BaseNetObj> netObj, bool read_enable, bool write_enable)
{
	return 0;
}

void Poll::destoryPoll()
{
	for (auto& thread : m_threads)
	{
		PostQueuedCompletionStatus(m_completionPort, 0, 0, nullptr);
	}

	for (auto& thread : m_threads)
	{
		thread.join();
	}

	CloseHandle(m_completionPort);
	m_network->NETDEBUG << "[IOCP] destoryPoll.";
}

#endif
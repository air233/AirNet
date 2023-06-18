#include "poll.h"

#ifdef _WIN32
#include "../network/network.h"
#include "../socket/socketops.h"
#include "../until/InetAddress.h"
#include "../../common/buffer/buffer.h"
#include <functional>
#include <iostream>
#include <MSWSock.h>

const int MAX_ACCPET_SIZE = 511;
constexpr int MAX_BUFFER_SIZE = 1024;

enum IOType
{
	IONone,
	IOAccept,
	IORecv,
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

void Poll::PostAccept(std::shared_ptr<BaseNetObj> netObj)
{
	SOCKET clientSock = createTCPSocket(netObj->localAddress().family());

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
	ioContext->type = IOAccept;

	DWORD dwByteRcv = 0;
	while (false == AcceptEx(netObj->fd(), clientSock, &ioContext->Buffer,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&dwByteRcv,
		(LPOVERLAPPED)ioContext))
	{
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			break;
		}
		m_network->NETERROR << "PostAccept AcceptEx Failed. Error: " << GetLastError() << std::endl;
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
			m_network->NETDEBUG << "[IOCP] work thread quit.";
			break;
		}

		if (result == FALSE)
		{
			int32_t error = GetLastError();

			m_network->NETERROR << "Failed to get completion status. Error: " << error << std::endl;

			if (error == WAIT_TIMEOUT || error == ERROR_NETNAME_DELETED)
			{
				
				//TODO:处理连接断开

				delete overlapped;
				
				//closesocket()

			}

			continue;
		}

		IO_CONTEXT* ioContext = reinterpret_cast<IO_CONTEXT*>(overlapped);

		m_network->NETDEBUG << "[IOCP] WorkerThread1 process event";



		if (ioContext->type == IOAccept)
		{
			//TODO:linsten fd 可读
			if (m_network->getNetMode() == TCP)
			{
				m_network->NETDEBUG << "[IOCP] tcp accept" << ioContext->NetID;

				//继续投递一个Accpet
				PostAccept(m_network->getServerNetObj());

				//TODO:处理接收客户端
				//clientSock关联listenSock
				SOCKET listenSocket = m_network->getServerNetObj()->fd();
				setsockopt(ioContext->Socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenSocket, sizeof SOCKET);
				
				//生成一个NetObj
				auto clientNetObj = m_network->makeNetObj(m_network, ioContext->Socket);
				if (false == m_network->insertNetObj(clientNetObj))
				{
					//TODO:插入失败处理
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

				memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
				ioContext->type = IORecv;
				ioContext->NetID = clientNetObj->getNetID();
				ioContext->DataBuf.buf = ioContext->Buffer;
				ioContext->DataBuf.len = sizeof(ioContext->Buffer);
				
				//与完成端口进行关联
				CreateIoCompletionPort((HANDLE)ioContext->Socket, m_completionPort, 0, 0);

				DWORD dwRecv, dwFlag = 0;
				//投递一个接收事件
				auto ret = WSARecv(ioContext->Socket, &ioContext->DataBuf, 1, &dwRecv, &dwFlag, &(ioContext->Overlapped), 0);
				if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
				{
					m_network->NETDEBUG << "[IOCP] client WSARecv fail. error:" << WSAGetLastError();
					//TODO:资源处理
				}
			}
			else if (m_network->getNetMode() == UDP)
			{
				//TODO:不存在
			}
			else if (m_network->getNetMode() == KCP)
			{
				//TODO:不存在
			}
		}
		else if(ioContext->type == IORecv)
		{
			if (bytesTransferred == 0)
			{
				// 客户端关闭连接
				// TODO:关闭事件
				m_network->NETDEBUG << "[IOCP] WorkerThread1 process event delete";
				delete ioContext;

				continue;
			}

			auto netObj = m_network->getNetObj2(ioContext->NetID);

			if (netObj == nullptr)
			{
				m_network->NETERROR << "not find net obj." << ioContext->NetID;

				delete ioContext;
				continue;
			}

			// 处理收到的数据
			m_network->NETDEBUG << "Received: " << ioContext->Buffer << std::endl;
		
			{
				//放入output buffer
				std::lock_guard<std::mutex> gurad(netObj->outputMutex());
				netObj->outputBuffer()->pushCString(ioContext->Buffer, bytesTransferred);
			}

			//TODO: 产生一个RecvJob;


			//继续接收事件
			memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
			DWORD dwRecv, dwFlag = 0;
			//投递一个接收事件
			auto ret = WSARecv(ioContext->Socket, &ioContext->DataBuf, 1, &dwRecv, &dwFlag, &(ioContext->Overlapped), 0);
			if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				m_network->NETDEBUG << "[IOCP] client WSARecv fail. error:" << WSAGetLastError();
				//TODO:资源处理
			}

			//TODO:生成recv job
		}
	}
}

bool Poll::createPoll(Network* network)
{
	m_network = network;

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
	IO_CONTEXT* ioContext = new IO_CONTEXT();
	ioContext->Socket = netObj->fd();
	ioContext->NetID = netObj->getNetID();
	memset(ioContext->Buffer, 0, MAX_BUFFER_SIZE);
	ioContext->DataBuf.buf = ioContext->Buffer;
	ioContext->DataBuf.len = sizeof(ioContext->Buffer);

	//绑定完成端口
	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(netObj->fd()), m_completionPort, 0, 0) == nullptr)
	{
		m_network->NETERROR << "[IOCP] Failed to associate client socket with IOCP. Error: " << GetLastError() << std::endl;
		delete ioContext;
		return false;
	}


	if (netObj->getNetMode() == TCP && netObj->isListen())
	{
		m_network->NETDEBUG << "[IOCP] create accpet thread.";

		//预先生成 投递AccpetEx事件
		for (int i = 0; i < MAX_ACCPET_SIZE; i++)
		{
			PostAccept(netObj);
		}
	}

	{
		//std::lock_guard<std::mutex> guard(m_iocontexts_mutex);
		//m_iocontexts.insert(std::make_pair(netObj->fd(), ioContext));
	}

	//关注可读事件
	//if (IOCompletionKey == IOListen)
	{
		//DWORD bytesReceived = 0;  // 用于存储实际接收的字节数
		//DWORD flags = 0;         // 接收操作的标志
		//int ret = WSARecv(netObj->fd(), &ioContext->DataBuf, 1, &bytesReceived, &flags, &ioContext->Overlapped, nullptr);
		//m_network->NETDEBUG << "[IOCP] addPoll WSARecv ret:" << ret << ",error:" <<GetLastError();

	}

	//关注可写事件



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
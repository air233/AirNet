#include "poll.h"

#ifdef _WIN32
#include "../network/network.h"
#include "../socket/socketops.h"
#include "../until/InetAddress.h"
#include "../netobj/udpnetobj.h"
#include "../../common/buffer/buffer.h"
#include <functional>
#include <iostream>

const int MAX_ACCPET_SIZE = 511;
constexpr int MAX_BUFFER_SIZE = 1024;
constexpr int MAX_UDP_BUFFER_SIZE = 65536;

enum class IOType : int
{
	IONone,
	IOAccept,
	IORecv,
	IOSend,
	IOConn,
	IORecvFrom,
	IOSendTo,
};

char* CIOType[7] =
{
	"None",
	"Accept",
	"Recv",
	"Send",
	"Conn",
	"RecvFrom",
	"SendTo",
};

struct IO_CONTEXT
{
	OVERLAPPED Overlapped;
	IOType type;
	SOCKET Socket;
	uint64_t NetID;
	WSABUF DataBuf;
	char Buffer[MAX_BUFFER_SIZE];
	
};

struct IO_UDP_CONTEXT
{
	OVERLAPPED Overlapped;
	IOType type;
	SOCKET Socket;
	uint64_t NetID;
	WSABUF DataBuf;
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
			delete ioContext;
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
			delete ioContext;
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
	else if (netObj->getNetMode() == (int)NetMode::UDP)
	{
		return PostRecvFrom(netObj);
	}

	return false;
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
			delete ioContext;
			return false;
		}
		m_network->NETDEBUG << "[IOCP] PostRecv.";
		return true;
	}

	return true;
}

bool Poll::PostRecvFrom(std::shared_ptr<BaseNetObj> netObj)
{
	IO_UDP_CONTEXT* ioUDPContext = new IO_UDP_CONTEXT();
	ioUDPContext->NetID = netObj->getNetID();
	ioUDPContext->Socket = netObj->fd();
	ioUDPContext->DataBuf.buf = new char[MAX_UDP_BUFFER_SIZE];
	memset(ioUDPContext->DataBuf.buf, 0, MAX_UDP_BUFFER_SIZE);
	ioUDPContext->DataBuf.len = MAX_UDP_BUFFER_SIZE;
	ioUDPContext->type = IOType::IORecvFrom;

	DWORD flags = 0;
	DWORD bytesReceived;

	sockaddr_storage addr;
	int addr_len = sizeof sockaddr_storage;

	int ret = WSARecvFrom(netObj->fd(), &ioUDPContext->DataBuf, 1, &bytesReceived, &flags, (sockaddr*)&addr, &addr_len, (LPOVERLAPPED)ioUDPContext, 0);
	InetAddress inetAddr(addr);
	m_network->NETDEBUG << "[IOCP] PostRecvFrom addr:" << inetAddr.toIpPort();
	
	if (ret != 0)
	{
		int32_t err = GetLastError();

		if (err != WSA_IO_PENDING)
		{
			PostErrorJob(netObj, err);
			delete ioUDPContext->DataBuf.buf;
			delete ioUDPContext;
			return false;
		}
		m_network->NETDEBUG << "[IOCP]1 PostRecvFrom.";
		return true;
	}

	return true;
}

bool Poll::PostSendEvent(std::shared_ptr<BaseNetObj> netObj)
{
	if (netObj->getNetMode() == (int)NetMode::TCP)
	{
		return PostSend(netObj);
	}
	else if (netObj->getNetMode() == (int)NetMode::UDP)
	{
		return PostSendTo(netObj);
	}

	return false;
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
			delete ioContext;
			return false;
		}

		//std::cout << "[IOCP] PostSend." << std::endl;
		m_network->NETDEBUG << "[IOCP] PostSend.";
		return true;
	}
	return true;
}

bool Poll::PostSendTo(std::shared_ptr<BaseNetObj> netObj)
{
	Message msg;
	bool ret = netObj->getMessage(msg);
	if (ret == false)
	{
		std::cout << "[IOCP] SendTo over." << std::endl;
		return true;
	}
	std::cout << "[IOCP] PostSendTo.msg:" << msg.m_message << std::endl;

	IO_UDP_CONTEXT* ioUDPContext = new IO_UDP_CONTEXT();
	ioUDPContext->NetID = netObj->getNetID();
	ioUDPContext->Socket = netObj->fd();
	ioUDPContext->DataBuf.buf = new char[MAX_UDP_BUFFER_SIZE];
	memset(ioUDPContext->DataBuf.buf, 0, MAX_BUFFER_SIZE);
	memcpy(ioUDPContext->DataBuf.buf, msg.m_message.c_str(), msg.m_message.size());
	ioUDPContext->DataBuf.len = MAX_UDP_BUFFER_SIZE;
	ioUDPContext->type = IOType::IOSendTo;

	DWORD lpNumberOfBytesSent;
	sockaddr* addr = netObj->peerAddress().getSockAddr();
	int addr_len = netObj->peerAddress().getSockAddrLen();

	int sendret = WSASendTo(netObj->fd(), &ioUDPContext->DataBuf, 1, &lpNumberOfBytesSent, 0, addr, addr_len,(LPOVERLAPPED)ioUDPContext, 0);
	if (sendret != 0)
	{
		int32_t err = GetLastError();

		if (err != WSA_IO_PENDING)
		{
			PostErrorJob(netObj, err);
			delete ioUDPContext->DataBuf.buf;
			delete ioUDPContext;
			return false;
		}

		std::cout << "[IOCP] PostSendTo." << std::endl;
		//m_network->NETDEBUG << "[IOCP] PostSendTo.";
		return true;
	}
	return true;
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
		m_network->NETDEBUG << "[IOCP] WorkerThread1 process event:" << CIOType[(int)ioContext->type] << "," << (int)ioContext->type;

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

				//投递一个NewConnectJob
				PostNewConnectJob(clientNetObj);
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

		else if (ioContext->type == IOType::IOSend )
		{
			auto netObj = m_network->getNetObj2(ioContext->NetID);
			if (netObj == nullptr)
			{
				m_network->NETERROR << "[IOCP] send not find net obj." << ioContext->NetID;
				delete ioContext;
				continue;
			}

			//继续处理发送
			PostSendEvent(netObj);
		}

		else if (ioContext->type == IOType::IORecvFrom)
		{
			IO_UDP_CONTEXT* ioUDPContext = reinterpret_cast<IO_UDP_CONTEXT*>(overlapped);
			auto netObj = m_network->getNetObj2(ioUDPContext->NetID);
			if (netObj == nullptr)
			{
				m_network->NETERROR << "[IOCP] recvfrom not find net obj." << ioUDPContext->NetID;
				delete ioUDPContext->DataBuf.buf;
				delete ioUDPContext;
				continue;
			}

			sockaddr_storage addr;
			getPeerAddr(ioUDPContext->Socket, addr);
			InetAddress inetAddr(addr);
			m_network->NETERROR << "[IOCP] recvfrom addr:" << inetAddr.toIpPort();

			//投递一个RecvFrom事件
			PostRecvFromJob(netObj,InetAddress(addr), ioUDPContext->DataBuf.buf,bytesTransferred);

			//继续接收消息
			ioUDPContext->type = IOType::IORecvFrom;
			::memset(ioUDPContext->DataBuf.buf, 0, MAX_UDP_BUFFER_SIZE);
			ioUDPContext->DataBuf.len = MAX_UDP_BUFFER_SIZE;
			DWORD flags = 0;
			DWORD bytesReceived;
			int addr_len = sizeof sockaddr_storage;
			int ret = WSARecvFrom(netObj->fd(), &ioUDPContext->DataBuf, 1, &bytesReceived, &flags, (sockaddr*)&addr, &addr_len, (LPOVERLAPPED)ioContext, 0);
			if (ret != 0)
			{
				int32_t err = GetLastError();

				if (err != WSA_IO_PENDING)
				{
					PostErrorJob(netObj, err);
					delete ioUDPContext->DataBuf.buf;
					delete ioUDPContext;
				}
				m_network->NETDEBUG << "[IOCP] PostRecvFrom.";
			}
			continue;
		}
		else if (ioContext->type == IOType::IOSendTo)
		{
			IO_UDP_CONTEXT* ioUDPContext = reinterpret_cast<IO_UDP_CONTEXT*>(overlapped);

			auto netObj = m_network->getNetObj2(ioUDPContext->NetID);
			if (netObj == nullptr)
			{
				m_network->NETERROR << "[IOCP] sendto not find net obj." << ioUDPContext->NetID;
				delete ioUDPContext->DataBuf.buf;
				delete ioUDPContext;
				continue;
			}

			auto udpNetObj = std::static_pointer_cast<UDPNetObj>(netObj);
			if (netObj == nullptr)
			{
				m_network->NETERROR << "[IOCP] sendto not find net obj2." << ioContext->NetID;
				delete ioUDPContext->DataBuf.buf;
				delete ioUDPContext;
				continue;
			}

			Message msg;
			bool ret = udpNetObj->getMessage(msg);
			if (ret == false)
			{
				std::cout << "[IOCP] SendTo over2." << std::endl;
				continue;
			}
			memset(ioUDPContext->DataBuf.buf, 0, MAX_BUFFER_SIZE);
			memcpy(ioUDPContext->DataBuf.buf, msg.m_message.c_str(), msg.m_message.size());

			DWORD lpNumberOfBytesSent;
			sockaddr* addr = netObj->peerAddress().getSockAddr();
			int addr_len = netObj->peerAddress().getSockAddrLen();

			int retSend = WSASendTo(netObj->fd(), &ioUDPContext->DataBuf, 1, &lpNumberOfBytesSent, 0, addr, addr_len, (LPOVERLAPPED)ioContext, 0);
			if (retSend != 0)
			{
				int32_t err = GetLastError();

				if (err != WSA_IO_PENDING)
				{
					PostErrorJob(netObj, err);
					delete ioUDPContext->DataBuf.buf;
					delete ioUDPContext;
					continue;
				}
				//std::cout << "[IOCP] PostSend." << std::endl;
				m_network->NETDEBUG << "[IOCP] PostSendTo2.";
			}
			continue;
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

	//处理异步连接
	if (netObj->getNetStatus() == Connecting)
	{
		return PostConnectEvent(netObj);
	}
	
	if (netObj->getNetMode() == (int)NetMode::TCP)
	{
		//只有TCP需要Listen
		if (netObj->isListen())
		{
			//预先生成 投递AccpetEx事件
			for (int i = 0; i < MAX_ACCPET_SIZE; i++)
			{
				PostAcceptEvent(netObj);
			}

			return true;
		}
	}
	else if (netObj->getNetMode() == (int)NetMode::UDP)
	{
		return PostRecvEvent(netObj);
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
bool Poll::enableReadPoll(std::shared_ptr<BaseNetObj> netObj, bool enable)
{
	if (netObj == nullptr)
	{
		return false;
	}

	bool ret = true;

	if (enable)
	{
		ret = PostRecvEvent(netObj);
	}

	return ret;
}

bool Poll::enableWritePoll(std::shared_ptr<BaseNetObj> netObj, bool enable)
{
	if (netObj == nullptr)
	{
		return false;
	}

	bool ret = true;

	if (enable)
	{
		ret = PostSendEvent(netObj);
	}

	return ret;
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
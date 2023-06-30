#ifndef _WIN32
#include "poll.h"
#include "../network/network.h"
#include "../socket/socketops.h"
#include "../until/inetAddress.h"
#include "../netobj/udpnetobj.h"
#include "../../common/buffer/buffer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

bool Poll::createPoll(Network* network)
{
	m_network = network;

	m_events.resize(16);

	m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);

	if (m_epollFd < 0)
	{
		m_network->NETWARN << "[EPOLL] create EPOLL FD fail.";

		return false;
	}

	//stop fd:
	if (pipe(m_pipefd) == -1)
	{
		m_network->NETWARN << "[EPOLL] create pipe FD fail.";
		return false;
	}

	epoll_event et;
	et.data.u64 = 0;
	et.events = EPOLLIN | EPOLLHUP | EPOLLET;
	if (-1 == ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_pipefd[0], &et))
	{
		m_network->NETWARN << "[EPOLL] register stop fd fail.";
		return false;
	}

	//work thread:
	m_threads.emplace_back(&Poll::WorkerThread, this);

	m_network->NETINFO << "[EPOLL] start epoll work.";
}

void Poll::destoryPoll()
{
	m_run = false;

	int err = 0;
	write(m_pipefd[1], "stop", 4);

	for (auto& thread : m_threads)
	{
		if (thread.joinable())
			thread.join();
	}

	::close(m_epollFd);

	m_network->NETINFO << "[EPOLL] destoryPoll.";
}

bool Poll::addPoll(std::shared_ptr<BaseNetObj>& netObj)
{
	if (netObj == nullptr) return false;

	epoll_event et;
	et.data.u64 = netObj->getNetID();

	if (netObj->getNetStatus() == Connecting)
	{
		et.events = EPOLLOUT | EPOLLET | EPOLLHUP;
	}
	else
	{
		et.events = EPOLLIN | EPOLLET | EPOLLHUP;
	}

	int ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, netObj->fd(), &et);

	if (ret == 0)
	{
		m_network->NETDEBUG << "[EPOLL] addPoll success, net id:" << netObj->getNetID();
	}

	return  ret == 0;
}

void Poll::delPoll(std::shared_ptr<BaseNetObj>& netObj)
{
	if (netObj == nullptr) return;

	::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, netObj->fd(), NULL);
}

bool Poll::enableReadPoll(std::shared_ptr<BaseNetObj>& netObj, bool enable)
{
	epoll_event et;
	et.data.u64 = netObj->getNetID();

	if (enable)
		et.events |= EPOLLIN;
	else
		et.events &= ~EPOLLIN;

	int ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, netObj->fd(), &et);

	if (ret == 0)
	{
		m_network->NETDEBUG << "[EPOLL] enableReadPoll success, net id:" << netObj->getNetID();
	}

	return  ret == 0;
}

bool Poll::enableWritePoll(std::shared_ptr<BaseNetObj>& netObj, bool enable)
{
	if (enable)
	{
		if (netObj->getNetMode() == (int)NetMode::TCP)
		{
			size_t len = 0;
			{
				std::lock_guard<std::mutex> lock(netObj->inputMutex());
				len = netObj->inputBuffer()->size();
			}

			if (len == 0)
			{
				return true;
			}
		}
		else if (netObj->getNetMode() == (int)NetMode::UDP)
		{
			if (0 == netObj->getMessageSize())
			{
				return true;
			}
		}
	}

	epoll_event et;
	et.data.u64 = netObj->getNetID();

	if (enable)
		et.events |= EPOLLOUT;
	else
		et.events &= ~EPOLLOUT;

	int ret = ::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, netObj->fd(), &et);

	if (ret == 0)
	{
		m_network->NETDEBUG << "[EPOLL] enableReadPoll success, net id:" << netObj->getNetID();
	}

	return  ret == 0;

}

void Poll::WorkerThread()
{
	while (true)
	{
		int numEvents = ::epoll_wait(m_epollFd, &*m_events.begin(), (int)m_events.size(), -1);
		
		if (numEvents < 0)
		{
			m_network->NETERROR << "[EPOLL] epoll wait error:" << errno;

			if (errno != EINTR)
			{
				break;
			}
		}

		if (numEvents > 0)
		{
			if (false == processEvent(m_events, numEvents))
			{
				break;
			}

			if ((size_t)numEvents == m_events.size())
			{
				m_events.resize(m_events.size() * 2);
			}
		}
	}

	m_network->NETINFO << "[EPOLL] work thread quit.";
}

bool Poll::processConnectEvent(std::shared_ptr<BaseNetObj>& netObj)
{
	//	get socket info
	sockaddr_storage local;
	getLocalAddr(netObj->fd(), local);

	InetAddress addr(local);
	netObj->setlocalAddress(addr);

	PostConnectJob(netObj, NET_SUCCESS);

	// reset event
	delPoll(netObj);

	return true;
}

bool Poll::processAcceptEvent(std::shared_ptr<BaseNetObj>& netObj)
{
	int32_t err = 0;
	sockaddr_storage addr;
	int32_t addr_len = sizeof(sockaddr_storage);

	while (true)
	{
		SOCKET client_fd = acceptSocket(netObj->fd(), (sockaddr*)&addr, addr_len, err);

		if (client_fd < 0)
		{
			if (err == EWOULDBLOCK || err == EAGAIN)
			{
				break;
			}
			else
			{
				m_network->NETERROR << "[EPOLL] acceptSocket fail:" << err;
				//TODO:should close?
				return false;
			}
		}
		else
		{
			auto clientNetObj = m_network->makeNetObj(m_network, client_fd);
			if (false == m_network->insertNetObj(clientNetObj))
			{
				m_network->NETERROR << "[EPOLL] Failed to insert netObj. net id: " << clientNetObj->getNetID();
				closeSocket(client_fd);
				continue;
			}
			m_network->NETDEBUG << "[EPOLL] TCP accept:" << clientNetObj->getNetID();

			sockaddr_storage addr;
			if (0 == getLocalAddr(client_fd, addr))
			{
				InetAddress localAddr(addr);
				clientNetObj->setlocalAddress(localAddr);
			}
			if (0 == getPeerAddr(client_fd, addr))
			{
				InetAddress peerAddr(addr);
				clientNetObj->setpeerAddress(peerAddr);
			}

			PostNewConnectJob(clientNetObj);
		}
	}
	return true;
}

bool Poll::processReadEvent(std::shared_ptr<BaseNetObj>& netObj)
{
	int32_t err = 0;
	bool isDisconnect = false;
	bool isError = false;

	if (netObj->isListen())
	{
		return processAcceptEvent(netObj);
	}

	if (netObj->getNetMode() == (int)NetMode::TCP)
	{
		Buffer buff;
		char tempBuf[1024];
		ssize_t read_size = 0;
		while (true)
		{
			::memset(tempBuf, 0, sizeof(tempBuf));
			read_size = readSocket(netObj->fd(), tempBuf, sizeof(tempBuf), err);

			if (read_size > 0)
			{
				//std::cout << "processReadEvent buff:" << tempBuf << std::endl;
				buff.pushCString(tempBuf, read_size);
			}
			else if (read_size == 0)
			{
				// disconnet
				isDisconnect = true;
				break;
			}
			else
			{
				if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR)
				{
					break;
				}
				else
				{
					// error
					isError = true;
					break;
				}
			}
		}

		if (buff.size() != 0)
		{
			PostRecvJob(netObj, buff.begin(), buff.size());
		}

		if (isDisconnect)
		{
			PostDisConnectJob(netObj, 0);
		}

		if (isError)
		{
			PostErrorJob(netObj, err);
		}

	}
	else if (netObj->getNetMode() == (int)NetMode::UDP)
	{
		char buff[65536] = { 0 };

		while (true)
		{
			sockaddr_storage addr;
			socklen_t addr_len = (socklen_t)sizeof(sockaddr_storage);
			ssize_t read_size = readFromSocket(netObj->fd(), buff, sizeof buff, (sockaddr*)&addr, &addr_len, err);

			if (read_size > 0)
			{
				InetAddress peerAddr(addr);
				PostRecvFromJob(netObj, peerAddr, buff, read_size);
				continue;
			}
			else if (read_size == 0)
			{
				// disconnet
				isDisconnect = true;
				break;
			}
			else
			{
				if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR)
				{
					break;
				}
				else
				{
					// error
					isError = true;
					break;
				}
			}
		}

		if (isDisconnect)
		{
			PostDisConnectJob(netObj, 0);
		}

		if (isError)
		{
			PostErrorJob(netObj, err);
		}
	}

	return true;
}

bool Poll::processWriteEvent(std::shared_ptr<BaseNetObj>& netObj)
{
	int32_t err = 0;

	if (netObj->getNetMode() == (int)NetMode::TCP)
	{
		Buffer tempBuf;
		{
			std::lock_guard<std::mutex> lock(netObj->inputMutex());
			tempBuf.swap(*netObj->inputBuffer());
		}

		ssize_t write_index = 0;

		write_index = writeSocket(netObj->fd(), tempBuf.begin(), tempBuf.size(), err);

		if (write_index == -1)
		{
			if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR)
			{
				write_index = 0;
			}
			else
			{
				PostErrorJob(netObj, err);
				return true;
			}
		}

		if (write_index < tempBuf.size())
		{
			int remain = tempBuf.size() - write_index;
			{
				std::lock_guard<std::mutex> lock(netObj->inputMutex());
				netObj->inputBuffer()->insert(tempBuf.begin() + write_index, remain);
			}
		}
		else
		{
			enableWritePoll(netObj, false);
		}

	}
	else if (netObj->getNetMode() == (int)NetMode::UDP)
	{
		Message msg;
		ssize_t send_size = 0;
		while (true)
		{
			if (false == netObj->getMessage(msg))
			{
				enableWritePoll(netObj, false);
				break;
			}

			send_size = writeToSocket(netObj->fd(), msg.m_message.c_str(), msg.m_message.size(),
				(const sockaddr*)msg.m_addr.getSockAddr(), msg.m_addr.getSockAddrLen(), err);

			if (send_size < 0)
			{
				if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR)
				{
					break;
				}

				m_network->NETERROR << "[EPOLL] write to socket err:" << err;
				//PostErrorJob(netObj, err);
				return true;
			}
		}
	}
	return true;
}

bool Poll::processHupEvent(std::shared_ptr<BaseNetObj>& netObj)
{
	std::cout << "[EPOLL] process hup event " << std::endl;
	PostDisConnectJob(netObj, 0);
}

bool Poll::processEvent(std::vector<struct epoll_event>& events, int size)
{
	//std::cout << "[EPOLL] process event size:" << size << std::endl;
	//m_network->NETDEBUG << "[EPOLL] process event size:"<< size; 

	for (int i = 0; i < size; i++)
	{
		//sotp fd
		if (events[i].data.u64 == 0)
		{
			std::cout << "[EPOLL] processEvent stop. fd:" << events[i].data.fd << std::endl;
			return false;
		}

		auto netObj = m_network->getNetObj2(events[i].data.u64);

		if (netObj == nullptr) continue;

		if (netObj->getNetStatus() == Connecting)
		{
			processConnectEvent(netObj);

			continue;
		}

		if (events[i].events & EPOLLIN)
		{
			processReadEvent(netObj);
		}

		if (events[i].events & EPOLLOUT)
		{
			processWriteEvent(netObj);
		}

		if (events[i].events & EPOLLHUP)
		{
			processHupEvent(netObj);
		}
	}

	return true;
}

#endif
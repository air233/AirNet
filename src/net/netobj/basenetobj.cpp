#include "basenetobj.h"
#include "../nettype.h"
#include "../socket/socketops.h"

BaseNetObj::BaseNetObj(uint64_t net_id, SOCKET fd):
	m_net_id(net_id),
	m_fd(fd),
	m_net_mode((int)NetMode::NONE),
	m_net_state(Disconnected),
	m_error(0),
	m_listen(0),
	m_io_type(IOREAD),
	m_network(nullptr)
{

}

BaseNetObj::~BaseNetObj()
{
	if (m_fd != INVALID_SOCKET)
	{
		closeSocket(m_fd);
		m_fd = INVALID_SOCKET;
	}

	m_net_state = Disconnected;
}

uint64_t BaseNetObj::getNetID()
{
	return m_net_id;
}

uint32_t BaseNetObj::getNetMode()
{
	return m_net_mode;
}

uint32_t BaseNetObj::getNetStatus()
{
	return m_net_state;
}

InetAddress& BaseNetObj::localAddress()
{
	return m_localAddr;
}

InetAddress& BaseNetObj::peerAddress()
{
	return m_peerAddr;
}

std::mutex& BaseNetObj::inputMutex()
{
	return m_input_mutex;
}

Buffer* BaseNetObj::inputBuffer()
{
	return &m_input_buf;
}

std::mutex& BaseNetObj::outputMutex()
{
	return m_output_mutex;
}

Buffer* BaseNetObj::outputBuffer()
{
	return &m_output_buf;
}

bool BaseNetObj::getMessage(Message& msg)
{
	return false;
}

SOCKET BaseNetObj::fd()
{
	return m_fd;
}

int32_t BaseNetObj::getError()
{
	return m_error;
}

void BaseNetObj::setError(int32_t error)
{
	m_error = error;
}

bool BaseNetObj::connect(InetAddress& address, uint64_t outms)
{
	m_peerAddr = address;

	return false;
}

bool BaseNetObj::asynConnect(InetAddress& address, uint64_t outms)
{
	m_peerAddr = address;

	return false;
}

bool BaseNetObj::bind(InetAddress& address)
{
	m_localAddr = address;

	return true;
}

bool BaseNetObj::listen()
{
	m_listen = 1;

	return true;
}

bool BaseNetObj::send(const char* data, size_t len)
{
	return false;
}

bool BaseNetObj::sendTo(InetAddress& address, const char* data, size_t len)
{
	return false;
}

bool BaseNetObj::isListen()
{
	return m_listen == 1;
}

bool BaseNetObj::close()
{
	return false;
}

void BaseNetObj::haveRead(bool enable)
{
	if (enable)
	{
		m_io_type |= IOREAD;
	}
	else
	{
		m_io_type &= ~IOREAD;
	}
}

void BaseNetObj::haveWrite(bool enable)
{
	if (enable)
	{
		m_io_type |= IOWRIT;
	}
	else
	{
		m_io_type &= ~IOWRIT;
	}
}

void BaseNetObj::haveAll(bool enable)
{
	if (enable)
	{
		m_io_type |= IOREADWRIT;
	}
	else
	{
		m_io_type &= ~IOREADWRIT;
	}
}

uint8_t BaseNetObj::getIOType()
{
	return m_io_type;
}

void BaseNetObj::setNetStatus(uint32_t status)
{
	m_net_state = status;
}

void BaseNetObj::setlocalAddress(InetAddress& localAddr)
{
	m_localAddr = localAddr;
}

void BaseNetObj::setpeerAddress(InetAddress& peerAddr)
{
	m_peerAddr = peerAddr;
}

void BaseNetObj::setNetwork(Network* network)
{
	m_network = network;
}


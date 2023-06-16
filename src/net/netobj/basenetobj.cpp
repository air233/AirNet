#include "basenetobj.h"
#include "../nettype.h"
#include "../socket/socketops.h"

BaseNetObj::BaseNetObj(uint64_t net_id, SOCKET fd):
	m_net_id(net_id),
	m_fd(fd),
	m_net_mode(NONE),
	m_net_state(Disconnected),
	m_error(0),
	m_listen(0),
	m_network(nullptr)
{

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

const InetAddress& BaseNetObj::localAddress()
{
	return m_localAddr;
}

const InetAddress& BaseNetObj::peerAddress()
{
	return m_peerAddr;
}

Buffer* BaseNetObj::inputBuffer()
{
	return &m_input_buf;
}

Buffer* BaseNetObj::outputBuffer()
{
	return &m_output_buf;
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

bool BaseNetObj::isListen()
{
	return m_listen;
}

bool BaseNetObj::close()
{
	return false;
}

//SOCKET BaseNetObj::accept(int32_t& err)
//{
//	err = 0;
//	return INVALID_SOCKET;
//}

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


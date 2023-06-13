#include "basenetobj.h"
#include "../nettype.h"

BaseNetObj::BaseNetObj(uint64_t net_id):
	m_net_id(net_id),
	m_fd(-1),
	m_net_mode(NONE),
	m_net_state(Disconnected),
	m_error(0),
	m_listen(0)
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

int32_t BaseNetObj::fd()
{
	return m_fd;
}

int32_t BaseNetObj::getError()
{
	return m_error;
}

bool BaseNetObj::connect(InetAddress& address, uint64_t outms)
{
	m_peerAddr = address;

	return false;
}

bool BaseNetObj::listen(InetAddress& address)
{
	m_localAddr = address;

	m_listen = 1;

	return false;
}

void BaseNetObj::send(const char* data, size_t len)
{
	m_input_buf.pushCString(data, len);
}

bool BaseNetObj::isListen()
{
	return m_listen;
}


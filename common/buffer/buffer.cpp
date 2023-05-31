#include "buffer.h"
#include "../until/encoding.h"

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
}

size_t Buffer::size()
{
    return m_buffer.size();
}

void Buffer::pushInt64(int64_t data)
{
    data = HostToBigEndian(data);

    push(data);
}

void Buffer::pushUint64(uint64_t data)
{
    data = HostToBigEndian((int64_t)data);

    push(data);
}

void Buffer::pushInt32(int32_t data)
{
    data = HostToBigEndian(data);

    push(data);
}

void Buffer::pushUint32(uint32_t data)
{
    data = HostToBigEndian((int32_t)data);

    push(data);
}

void Buffer::pushInt16(int16_t data)
{
    data = HostToBigEndian(data);

    push(data);
}

void Buffer::pushUint16(uint16_t data)
{
	data = HostToBigEndian((int16_t)data);

	push(data);
}

void Buffer::pushInt8(int8_t data)
{
    push(data);
}

void Buffer::pushUint8(uint8_t data)
{
    push(data);
}

void Buffer::pushString(std::string& str)
{
    push(str.size());
    push(str.c_str(), str.size());
}

void Buffer::pushCString(const char* cstr, size_t size)
{
	push(size);
	push(cstr, size);
}

bool Buffer::peekInt64(int64_t& data)
{
	if (false == peek(data))
		return false;

    data = BigEndianToHost(data);
	return true;
}

bool Buffer::peekUint64(uint64_t& data)
{
	if (false == peek(data))
		return false;

    data = (uint64_t)BigEndianToHost((int64_t)data);
    return true;
}

bool Buffer::peekInt32(int32_t& data)
{
	if (false == peek(data))
		return false;

	data = BigEndianToHost(data);
	return true;
}

bool Buffer::peekUint32(uint32_t& data)
{
	if (false == peek(data))
		return false;

	data = (uint32_t)BigEndianToHost((int32_t)data);
	return true;
}

bool Buffer::peekInt16(int16_t& data)
{
	if (false == peek(data))
		return false;

	data = BigEndianToHost(data);
	return true;
}

bool Buffer::peekUint16(uint16_t& data)
{
	if (false == peek(data))
		return false;

	data = (uint16_t)BigEndianToHost((int16_t)data);
	return true;
}

bool Buffer::peekInt8(int8_t& data)
{
	if (false == peek(data))
		return false;

	return true;
}

bool Buffer::peekUint8(uint8_t& data)
{
	if (false == peek(data))
		return false;

	return true;
}

void Buffer::peekString(std::string& str, size_t size)
{
    str = get(size);
}

void Buffer::peekCString(std::string& str, size_t size)
{
    str = get(size);
}

void Buffer::push(const char* data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        m_buffer.push_back(data[i]);
    }
}

std::string Buffer::get(size_t size)
{
    if (m_buffer.size() == 0)
        return dummy;

    if (size > m_buffer.size())
    {
        size = m_buffer.size();
    }

    std::string data(m_buffer.begin(), m_buffer.begin() + size);

    m_buffer.erase(m_buffer.begin(), m_buffer.begin() + size);

    return data;
}

void Buffer::swap(Buffer& rsh)
{
    m_buffer.swap(rsh.m_buffer);
}

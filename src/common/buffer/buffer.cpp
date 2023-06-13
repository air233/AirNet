#include "buffer.h"
#include <assert.h>
#include "../until/encoding.h"
#include <iostream>

Buffer::Buffer(size_t init_size) :
    m_buffer(init_size),
    m_read_index(0),
    m_write_index(0)
{

}

Buffer::~Buffer()
{
}


void Buffer::pushInt64(int64_t data)
{
    data = HostToBigEndian(data);

    push(data);
}

void Buffer::pushUint64(uint64_t data)
{
    data = HostToBigEndian(data);

    push(data);
}

void Buffer::pushInt32(int32_t data)
{
    data = HostToBigEndian(data);

    push(data);
}

void Buffer::pushUint32(uint32_t data)
{
    data = HostToBigEndian(data);

    push(data);
}

void Buffer::pushInt16(int16_t data)
{
    data = HostToBigEndian(data);

    push(data);
}

void Buffer::pushUint16(uint16_t data)
{
	data = HostToBigEndian(data);

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

void Buffer::pushString(const std::string& str)
{
    push(str.c_str(), str.size());
}

void Buffer::pushCString(const char* cstr, size_t size)
{
	push(cstr, size);
}

void Buffer::insertCString(const char* cstr, size_t start_index, size_t size)
{
    insert(cstr + start_index, size);
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

    data = BigEndianToHost(data);
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

	data = BigEndianToHost(data);
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

	data = BigEndianToHost(data);
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

size_t Buffer::peekString(std::string& str, size_t size)
{
    str = get(size);

    return str.size();
}

size_t Buffer::peekCString(std::string& str, size_t size)
{
    str = get(size);

    return str.size();
}

size_t Buffer::peekCString(char* data, size_t size)
{
   std::string str = get(size);

   memcpy(data, str.c_str(), str.size());

   return str.size();
}

void Buffer::append(const Buffer& buf)
{
    pushCString(buf.begin() + m_read_index, buf.readableSize());
}

void Buffer::drop(size_t size)
{
    m_read_index += size;
    if (m_read_index > m_write_index)
    {
        m_read_index = m_write_index;
    }
}

void Buffer::dropAll()
{
    m_read_index = 0;
    m_write_index = 0;
}

size_t Buffer::readableSize() const
{
    return m_write_index - m_read_index;
}

size_t Buffer::writableSize() const
{
    return m_buffer.size() - m_write_index;
}

void Buffer::ensureWritableSize(size_t len)
{
    if (writableSize() < len)
    {
        makeSpace(len);
    }
    assert(writableSize() >= len);
}

void Buffer::makeSpace(size_t len)
{
    size_t space = m_read_index + writableSize();

    if (space < len)
    {
        m_buffer.resize(m_write_index + len);
    }
    else
    {
        size_t readable = readableSize();
        memcpy(begin(), begin() + m_read_index, readable);
        m_read_index = 0;
        m_write_index = readable;
    }
}

char* Buffer::begin()
{
    return m_buffer.data();
}

const char* Buffer::begin() const
{
    return m_buffer.data();
}

void Buffer::push(const void* data, size_t size)
{
    ensureWritableSize(size);
    
    memcpy(begin() + m_write_index, data, size);

    m_write_index += size;
}

void Buffer::insert(const void* data, size_t size)
{
    if (m_read_index >= size)
    {
        memcpy(begin() + m_read_index - size, data, size);
        m_read_index -= size;
    }
    else
    {
        Buffer newbuff;
        newbuff.push(data,size);
        newbuff.push(begin() + m_read_index, readableSize());
        swap(newbuff);
    }
}

std::string Buffer::get(size_t size)
{
    size_t read_size = readableSize() < size ? readableSize() : size;

    std::string data(begin()+m_read_index, begin()+m_read_index+ read_size);

    m_read_index += read_size;

    return data;
}

size_t Buffer::size()
{
    return readableSize();
}

void Buffer::swap(Buffer& rsh)
{
    m_buffer.swap(rsh.m_buffer);
    std::swap(m_read_index, rsh.m_read_index);
    std::swap(m_write_index, rsh.m_write_index);
}

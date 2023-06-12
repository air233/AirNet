/*Buffer*/
#pragma once
#include<string>
#include<vector>
#include<stddef.h>
#include <cstdint>

/*
简单的Buffer类:
添加数据时转换为大字端序
取出时转为主机序

TODO:目前不是线程安全,后期会拓展线程安全buffer
*/

static const size_t kInitSize = 2;

static std::string dummy("");

class Buffer
{
public:
    Buffer(size_t init_size = kInitSize);
    ~Buffer();

    void insert(const void* data, size_t size);
    void push(const void* data,size_t size);

    /*暂时不提供
    void pop(size_t size, char* data) {};*/

    /*返回一个string的容器*/
    std::string get(size_t size);

	template <typename T>
	void push(const T& data);

	template <typename T>
	bool peek(T& data);

    /*以下为API*/
    size_t size();
    void swap(Buffer& rsh);

	void pushInt64(int64_t data);
    void pushUint64(uint64_t data);
	void pushInt32(int32_t data);
    void pushUint32(uint32_t data);
	void pushInt16(int16_t data);
    void pushUint16(uint16_t data);
	void pushInt8(int8_t data);
    void pushUint8(uint8_t data);
    void pushString(std::string& str);
    void pushCString(const char* cstr, size_t size);
    void insertCString(const char* cstr,size_t start_index , size_t size);

    bool peekInt64(int64_t& data);
    bool peekUint64(uint64_t& data);
    bool peekInt32(int32_t& data);
    bool peekUint32(uint32_t& data);
    bool peekInt16(int16_t& data);
    bool peekUint16(uint16_t& data);
    bool peekInt8(int8_t& data);
    bool peekUint8(uint8_t& data);
    size_t peekString(std::string& str, size_t size);
    size_t peekCString(std::string& str, size_t size);
    size_t peekCString(char* data, size_t size);

public:
    size_t readableSize();
    size_t writableSize();
    char* begin();
    const char* begin() const;

protected:
	void ensureWritableSize(size_t len);
	void makeSpace(size_t len);

private:
	std::vector<char> m_buffer;
    size_t m_read_index;
    size_t m_write_index;
};

template <typename T>
void Buffer::push(const T& data)
{
    push(&data, sizeof data);
}

template <typename T>
bool Buffer::peek(T& data)
{
    size_t sc = sizeof data;

    if (sc > readableSize())
        return false;

    std::string str = get(sc);

    data = *(reinterpret_cast<T*>((char*)str.data()));
    return true;
}

#pragma once
#include <cstdint>

enum SOCKET_TYPE
{
    NONE,
    TCP,
    UDP,
    KCP,
    MAX,
};

enum
{
	max_buff = 10240,
};


class BaseSocket
{
public:
    BaseSocket();
    ~BaseSocket();

    /*初始化操作*/
    virtual int start() { return 0; }
    virtual int recv() { return 0; }
    virtual int send() { return 0; }

    virtual int getBuffSize() { return 0; }

protected:
    uint64_t m_netid;

    int m_fd;

    int m_socket_type;
};

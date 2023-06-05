#pragma once

enum SOCKET_TYPE
{
    TCP,
    UDP,
};

class BaseSocket
{
public:
    BaseSocket();
    ~BaseSocket();

    enum
    {
        max_buff = 10240,
    };

private:
    int m_fd;

    int m_socket_type;
};

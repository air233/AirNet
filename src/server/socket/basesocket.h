#pragma once
#include<stddef.h>
#include<vector>

class BaseSocket
{
public:
    BaseSocket();
    ~BaseSocket();

    enum
    {
        max_buff = 10240,
    };

    virtual size_t send();
    virtual size_t recv();

    //绑定地址
    bool bind();

//TODO:
//  1.设置套接字为非阻塞
//  2.设置套接字绑定地址可重用

    size_t push_sbuff(char* buff);
    size_t push_rbuff(char* buff);

private:
    int m_fd;
    int m_socket_type;
    
    std::vector<char> m_recv_buff;
    std::vector<char> m_send_buff;
};

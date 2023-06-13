#pragma once
#include <cstdint>
#include "../nettype.h"
#include "../../common/buffer/buffer.h"

class BaseSocket
{
public:
    BaseSocket();
    ~BaseSocket();

    virtual int recv() { return 0; }
    virtual int send() { return 0; }

protected:
    uint64_t m_netid;
    int m_fd;
    int m_socket_type;
    int m_error;


    Buffer m_input_buf;
    Buffer m_output_buf;
};
#pragma once
#include "basesocket.h"

class TCPSocket : public BaseSocket
{
public:
    TCPSocket();
    ~TCPSocket();

    bool listen();

    void setNoDelay(bool on);
    void setReuseAddr(bool on);
    void setKeepAlive(bool on);
};

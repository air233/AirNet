#pragma once
#include "basesocket.h"

class TCPSocket : public BaseSocket
{
public:
    TCPSocket();
    ~TCPSocket();

    bool listen();
};

#pragma once
#include "Socket/socket.h"
#include "Ip/ipendpoint.h"
#include "Network/network.h"
#include <iostream>


class Client{
public:
    Client(const MyRedis::IPVersion& ipversion);
    void connect(const MyRedis::IPEndpoint& ipendpoint);
    const bool isConnected();
    void Frame();

private:
    bool connected = false;
    MyRedis::Socket socket{};
};



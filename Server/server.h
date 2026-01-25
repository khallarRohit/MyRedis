#pragma once
#include "Socket/socket.h"
#include "Network/network.h"
#include "TCPConnection/tcpconnection.h";
#include <vector>
#include <iostream>

namespace MyRedis {

    class Server {
    public:
        Server(const IPEndpoint&& ipendpoint);
        void initialize();
        void frame();
        void onConnect();
        void onDisconnect();
        void closeConnection(int connectionIndex, std::string&& reason);

    private:
        Socket listeningSocket;
        std::vector<TCPConnection> connections;
        std::vector<WSAPOLLFD> fdList;
    };


}
#pragma once
#include <vector>
#include <iostream>
#include <thread>
#include "Socket/socket.h"
#include "Network/network.h"
#include "SharedContext/sharedlock.h"
#include "TCPConnection/tcpconnection.h"
#include "Error/WSAexception.h"

namespace MyRedis {

    class Server {
    public:
        // take a IPEndpoint for server and create the listening socket
        Server(const IPEndpoint&& ipendpoint);
        TCPConnection& createNewConnection();
        void initialize();
        void frame(int& failCount, bool& listeningSocketFailed);
        void onConnect();
        void onDisconnect();
        void closeConnection(int connectionIndex, std::string&& reason);
        void printServerInfo();
        int getPollFailCount();

    private:
        Socket listeningSocket;
        std::vector<TCPConnection> connections;
        std::vector<WSAPOLLFD> fdList;
        std::shared_ptr<SharedLock> ctx{nullptr};   
        int pollFailCount{5};  
    };


}
#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include "Socket/socket.h"
#include "Network/network.h"
#include "TCPConnection/tcpconnection.h"
#include "Error/WSAexception.h"

namespace MyRedis {

    class InQueue;
    class Dispatcher;
    class RedisDatabase;

    class Server {
    public:
        // take a IPEndpoint for server and create the listening socket
        Server(IPEndpoint&& ipendpoint);
        void initialize();
        void frame(int& failCount, bool& listeningSocketFailed);
        void closeConnection(int connectionIndex, std::string&& reason);
        void printServerInfo();
        int getPollFailCount();

    private:
        Socket listeningSocket;
        std::vector<TCPConnection> connections;
        std::vector<WSAPOLLFD> fdList;
        int pollFailCount{5};  

        Dispatcher& dispatcher;
        ThreadPool& pool;

        std::shared_ptr<RedisDatabase> database;
    };


}
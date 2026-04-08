#include "tcpconnection.h"

namespace MyRedis{

    TCPConnection::TCPConnection(Socket&& socket)
    :socket(std::make_unique<Socket>(std::move(socket))),
    packetManager(std::make_unique<PacketManager>())
    {}

    TCPConnection::TCPConnection()
    :socket(std::make_unique<Socket>()),
    packetManager(std::make_unique<PacketManager>())
    {}

    void TCPConnection::printClientInfo(){
        if(socket){
            socket->printSocketInfo();
        }
    }
}
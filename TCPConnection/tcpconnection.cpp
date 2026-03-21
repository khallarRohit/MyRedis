#include "tcpconnection.h"

namespace MyRedis{

    // TCPConnection::TCPConnection(const Socket& handle, std::shared_ptr<SharedLock> ctx)
    // :handle(handle), incommingPm(ctx){}

    // TCPConnection::TCPConnection(std::shared_ptr<SharedLock> ctx)
    // :handle(), incommingPm(ctx){}

    TCPConnection::TCPConnection(const Socket& socket, std::shared_ptr<SharedLock> ctx)
    :socket(std::make_unique<Socket>(socket)) {}

    TCPConnection::TCPConnection(std::shared_ptr<SharedLock> ctx)
    :socket(std::make_unique<Socket>()){}

    void TCPConnection::printClientInfo(){
        socket->printSocketInfo();
    }
}
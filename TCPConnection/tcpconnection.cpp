#include "tcpconnection.h"

namespace MyRedis{

    TCPConnection::TCPConnection(Socket& handle)
    :handle(handle){}

    void TCPConnection::_close(){
        handle._close();
    }

    void TCPConnection::printClientInfo(){
        handle.printSocketInfo();
    }
}
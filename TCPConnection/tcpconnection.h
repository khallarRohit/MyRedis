#pragma once
#include "Socket/socket.h"
// #include "Packet/packetmanager.h"
#include <memory>
#include <memory>
#include "SharedContext/sharedlock.h"

namespace MyRedis{
    class TCPConnection{
    public:
        
        // copies parameter socket into it's own.
        TCPConnection(const Socket& socket, std::shared_ptr<SharedLock> ctx);

        // create a socket of it's own
        TCPConnection(std::shared_ptr<SharedLock> ctx); 

        void printClientInfo();        
        std::unique_ptr<Socket> socket;
        // PacketManager incommingPm;        
    };
}

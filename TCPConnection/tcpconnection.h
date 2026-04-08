#pragma once
#include "Socket/socket.h"
#include "Packet/packetmanager.h"
#include <memory>
#include <memory>

namespace MyRedis{
    class TCPConnection{
    public:
        TCPConnection(Socket&& socket);

        // create a socket of it's own
        TCPConnection(); 

        ~TCPConnection() = default;

        TCPConnection(const TCPConnection&) = delete;
        TCPConnection& operator=(const TCPConnection&) = delete;

        TCPConnection(TCPConnection&&) noexcept = default;
        TCPConnection& operator=(TCPConnection&&) noexcept = default;

        void printClientInfo();        
        std::unique_ptr<Socket> socket;
        std::unique_ptr<PacketManager> packetManager;
    };
}

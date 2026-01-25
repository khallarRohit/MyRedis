#pragma once
#include "socket.h"

namespace MyRedis{
    class TCPConnection{
    public:
        TCPConnection(Socket& handle);   
        void _close();  
        void printClientInfo();        
        Socket& handle;
        
    private:
        
    };
}

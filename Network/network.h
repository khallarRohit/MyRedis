#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <iostream>
#include "Error/WSAexception.h"


namespace MyRedis{

class Network {

    public:
        // Constructor initializes Winsock
        Network();
        
        // Destructor shuts it down cleanly
        ~Network();

        // Delete copy/move semantics. There should only ever be ONE Network object.
        Network(const Network&) = delete;
        Network& operator=(const Network&) = delete;
        Network(Network&&) = delete;
        Network& operator=(Network&&) = delete;
    };

}
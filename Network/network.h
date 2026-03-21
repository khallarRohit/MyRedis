#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include "Error/WSAexception.h"


namespace MyRedis{

    class Network{
    public:
        static void initialize();
        static void shutdown();
    };

}
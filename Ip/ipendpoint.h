#pragma once
#include "ipversion.h"
#include "Error/error.h"
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <cstdint>


namespace MyRedis{

    class IPEndpoint {
    public:
        IPEndpoint(sockaddr* addr);
        IPEndpoint(const char* ip, const unsigned short port);
        sockaddr_in getSockaddrIPv4();
        sockaddr_in6 getSockaddrIPv6();
        IPVersion getIPVersion();
        std::string getIP();
        unsigned short getPort();
        std::string getNumericHost();
        uint8_t* getIPBytes();

    private:
        IPVersion ipversion{};
        std::string hostname{};
        std::string ipS{};
        unsigned short port{};
        uint8_t ipBytes[16]{}; 
    };

}
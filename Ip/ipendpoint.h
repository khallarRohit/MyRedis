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
        IPEndpoint();
        IPEndpoint(sockaddr* addr);
        IPEndpoint(const char* ip, const unsigned short port);
        sockaddr_in getSockaddrIPv4() const;
        sockaddr_in6 getSockaddrIPv6() const;
        IPVersion getIPVersion() const;
        std::string getIP() const;
        unsigned short getPort() const;
        std::string getNumericHost() const;
        uint8_t* getIPBytes() const;

    private:
        IPVersion ipversion{};
        std::string hostname{};
        std::string ipS{};
        unsigned short port{};
        uint8_t ipBytes[16]{}; 
    };

}
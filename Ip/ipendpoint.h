#pragma once
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <cstdint>
#include <array>
#include "Error/error.h"
#include "ipversion.h"
#include "Error/WSAexception.h"

// copy constructors
// deep copy 
// shallow copy
// move semantics

// = operator
// deep copy
// shallow copy
// move semantics



namespace MyRedis{

    class IPEndpoint {
    public:
        IPEndpoint(const IPVersion& ipversion); // constructor initializes to default values

        // copy constructors
        IPEndpoint(const IPEndpoint& ipendpoint) = default; 
        IPEndpoint& operator=(const IPEndpoint& ipendpoint) = default; 

        // construct IPEndpoint using info passed as parameters
        IPEndpoint(sockaddr* addr);
        IPEndpoint(const char* ip, const unsigned short port);

        // return a sockaddr_4, to be called when ipversion = IPv4
        sockaddr_in getSockaddrIPv4() const;

        // return a sockaddr_6, to be called when ip-version = IPv6
        sockaddr_in6 getSockaddrIPv6() const;

        // return the ip-version of ip-endpoint
        IPVersion getIPVersion() const;

        // return the ip of ip-endpoint
        std::string getIP() const;

        // return the port of ip-endpoint
        unsigned short getPort() const;

        // return the numeric-host of ip-endpoint
        std::string getNumericHost() const;

        // fills parameterized array wiht host ip-bytes
        std::array<uint8_t, 16> getIPBytes() const;

        bool getBound() const;

    private:
        IPVersion ipversion{IPVersion::unknown};
        std::string hostname{};
        std::string ipS{};
        unsigned short port{};
        std::array<uint8_t, 16> ipBytes;
        bool isBound = false; 
    };

}
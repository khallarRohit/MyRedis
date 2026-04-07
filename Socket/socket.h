#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <utility>
#include <iostream>
#include "IP/ipendpoint.h"
#include "Error/WSAexception.h"

namespace MyRedis{

    enum SocketOption{
        TCP_NoDelay, //TRUE = disable nagle's algorithm
		IPV6_Only, //TRUE = Only ipv6 can connect. FALSE = ipv4 and ipv6 can both connect.
    };

    class Socket{
        public:
            Socket(); // constructor initializes to default values
            ~Socket();

            Socket(const Socket& handle) = delete; // shallow copy
            Socket& operator=(const Socket& handle) = delete; // shallow assignment

            Socket(Socket&& handle) noexcept;  // move  
            Socket& operator=(Socket&& handle) noexcept;  // move

            // create a new instance of socket using the parameters passed
            Socket(const IPEndpoint& ipendpoint, SOCKET& skt); 
            Socket(const IPEndpoint& ipendpoint); 
            Socket(const IPVersion& ipversion);

            void _close();
            void _listen();
            void _connect(const IPEndpoint& ipendpoint);
            const bool _accept(Socket& handle);
            const bool checkBound();
            SOCKET getSocket();
            IPVersion getIPVersion();
            void printSocketInfo();
            ~Socket();

        private:
            void _bind();
            void setSocketOptions(const SocketOption& option, BOOL value);
            void setBlocking(const bool isBlocking);
            IPEndpoint ipendpoint{IPVersion::unknown};
            SOCKET skt = INVALID_SOCKET;
            bool isBound = false;
    };

}
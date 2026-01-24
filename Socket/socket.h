#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <utility>
#include "IP/ipendpoint.h"

namespace MyRedis{

    enum SocketOption{
        TCP_NoDelay, //TRUE = disable nagle's algorithm
		IPV6_Only, //TRUE = Only ipv6 can connect. FALSE = ipv4 and ipv6 can both connect.
    };

    class Socket{
        public:
            Socket(const IPEndpoint& ipendpoint);
            Socket(const IPEndpoint& ipendpoint, const SOCKET& skt);
            void _close();
            void _bind();
            void _listen(const int backlog);
            void _accept(SOCKET& skt, IPEndpoint& ipendpoint);


        private:
            void setSocketOptions(const SocketOption& option, BOOL value);
            void setBlocking(const bool isBlocking);
            IPEndpoint ipendpoint;
            SOCKET skt = INVALID_SOCKET;
    };

}
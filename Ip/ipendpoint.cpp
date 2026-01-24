#include "ipendpoint.h"

// ip -> NULL terminated ANSI string that contains a host (node) name or a numeric host address string.
// For the Internet protocol, the numeric host address string is a dotted-decimal IPv4 address or an IPv6 hex address. 

/*
// addrinfo
typedef struct addrinfo {
    int             ai_flags;
    int             ai_family;
    int             ai_socktype;
    int             ai_protocol;
    size_t          ai_addrlen;
    char            *ai_canonname;
    struct sockaddr *ai_addr;
    struct addrinfo *ai_next;
} ADDRINFOA, *PADDRINFOA;
*/

/*
// sockaddr_in
typedef struct sockaddr_in {
    short          sin_family;
    u_short        sin_port;
    struct in_addr sin_addr;
    char           sin_zero[8];
} SOCKADDR_IN, *PSOCKADDR_IN, *LPSOCKADDR_IN;
*/

/*
// struct in_addr {
//   union {
//     struct {
//       u_char s_b1;
//       u_char s_b2;
//       u_char s_b3;
//       u_char s_b4;
//     } S_un_b;
//     struct {
//       u_short s_w1;
//       u_short s_w2;
//     } S_un_w;
//     u_long S_addr;
//   } S_un;
// };

*/

namespace MyRedis{

    IPEndpoint::IPEndpoint(const char* ip, unsigned short port){
        this->port = port;
        addrinfo hints{};
        hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_protocol = IPPROTO_TCP;
        addrinfo* resultInfo = nullptr; // will contain pointer to a linked list
        // with one or more addrinfo struct that contains response information about the host.

        int res = getaddrinfo(ip, NULL, &hints, &resultInfo);
        
        if(res != 0){
            throw WSAGetLastError();
        }

        addrinfo* ptr = nullptr;
        // Optional: Loop to prefer IPv4 (since AF_UNSPEC might return IPv6 first)
        for (ptr = resultInfo; ptr != nullptr; ptr = ptr->ai_next) {
            if (ptr->ai_family == AF_INET) {
                break; // Found an IPv4 match, stop here
            }
        }

        // If no IPv4 found, fallback to the first result (likely IPv6)
        if (ptr == nullptr) {
            ptr = resultInfo;
        }

        if (ptr->ai_family == AF_INET){ 
            sockaddr_in* v4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
            ipversion = IPVersion::IPv4;
            memcpy(&ipBytes[0], &v4->sin_addr, sizeof(ULONG));
    
            ipS.resize(16);
            inet_ntop(AF_INET, &v4->sin_addr, &ipS[0], 16);            
        }else{
            sockaddr_in6* v6 = reinterpret_cast<sockaddr_in6*>(ptr->ai_addr);

            ipversion = IPVersion::IPv6;
            memcpy(&ipBytes[0], &v6->sin6_addr, 16);

            ipS.resize(46);
            inet_ntop(AF_INET6, &v6->sin6_addr, &ipS[0], 46);
        }  

        hostname = ip;
        freeaddrinfo(resultInfo);
    }

    IPEndpoint::IPEndpoint(sockaddr* addr){
        if (addr->sa_family == AF_INET){
            sockaddr_in * addrv4 = reinterpret_cast<sockaddr_in*>(addr);
            ipversion = IPVersion::IPv4;
            port = ntohl(addrv4->sin_port);
            memcpy(&ipBytes[0], &addrv4->sin_addr, sizeof(ULONG));
            ipS.resize(16);
            inet_ntop(AF_INET6, &addrv4->sin_addr, &ipS[0], 16);
        }else{
            sockaddr_in6 * addrv6 = reinterpret_cast<sockaddr_in6*>(addr);
            ipversion = IPVersion::IPv6;
            port = ntohs(addrv6->sin6_port);
            memcpy(&ipBytes[0], &addrv6->sin6_addr, 16);
            ipS.resize(46);
            inet_ntop(AF_INET6, &addrv6->sin6_addr, &ipS[0], 46);
        }
        hostname = ipS;
    }

    sockaddr_in IPEndpoint::getSockaddrIPv4(){
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htonl(port);
        memcpy(&addr.sin_addr, &ipBytes[0], sizeof(ULONG));
        return addr;
    }

    sockaddr_in6 IPEndpoint::getSockaddrIPv6(){
        sockaddr_in6 addr{};
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htonl(port);
        memcpy(&addr.sin6_addr, &ipBytes[0], sizeof(ULONG));
        return addr;
    }

    IPVersion IPEndpoint::getIPVersion(){
        return ipversion;
    }

    std::string IPEndpoint::getIP(){
        return ipS;
    }

    uint8_t* IPEndpoint::getIPBytes(){
        return ipBytes;
    }

    std::string IPEndpoint::getNumericHost(){
        return hostname;
    }

    unsigned short IPEndpoint::getPort(){
        return port;
    }

}


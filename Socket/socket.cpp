#include "socket.h"

/*
    SOCKET WSAAPI socket(
        [in] int af,
        [in] int type,
        [in] int protocol
    );

    af => family specification
    type => connection type
    protocol => connection protocol

*/

/*
    ioctlsocket() => control I/O mode of a socket
    int WSAAPI ioctlsocket(
        [in]      SOCKET s,
        [in]      long   cmd,
        [in, out] u_long *argp
    );

    s => socket descriptor
    cmd => command to perform on socket
    argp => pointer to parameter of cmd

    success => 0
*/

/*
    setsockopt() => to set a socket options

    int WSAAPI setsockopt(
        [in] SOCKET     s,
        [in] int        level,
        [in] int        optname,
        [in] const char *optval,
        [in] int        optlen
    );

    s => descriptor
    if no error => returns zero
    if error => value of SOCKET_ERROR is returned

*/

/*
    int WSAAPI closesocket(
        [in] SOCKET s
    );
    error => SOCKET_ERROR is returned
*/

/*
    bind() => associate a local address with a socket

    int WSAAPI bind(
        [in] SOCKET         s,
        [in] const sockaddr *name,
        [in] int            namelen
    );

*/

/*
    listen() => places socket in a state in which it is listening for incomming connections

    int WSAAPI listen(
        [in] SOCKET s,
        [in] int    backlog
    );

    s => unconnected descriptor
    backlog => The maximum length of the queue of pending connections.after which starts to reject connections 

*/

/*
    accept() => permit incomming connection attempt on a socket

    SOCKET WSAAPI accept(
        [in]      SOCKET   s,
        [out]     sockaddr *addr,
        [in, out] int      *addrlen
    );

    s => socket descriptor on which a attempt to connection is done
    addr => An optional pointer to a buffer that receives the address of the connecting entity, as known to the communications layer. 
    addrlen => An optional pointer to an integer that contains the length of structure pointed to by the addr parameter.

    If no error occurs, accept returns a value of type SOCKET that is a descriptor for the new socket.
    Otherwise, a value of INVALID_SOCKET is returned, and a specific error code can be retrieved by calling WSAGetLastError.

*/

namespace MyRedis{

    Socket::Socket(){};

    Socket::Socket(const IPEndpoint& ipendpoint)
    :ipendpoint(ipendpoint){
        IPVersion version = ipendpoint.getIPVersion();
        
        skt = socket((version == IPVersion::IPv4 ? AF_INET : AF_INET6), SOCK_STREAM, IPPROTO_TCP);
        if(skt == INVALID_SOCKET){
            throw WSAGetLastError();
        }
        
        setBlocking(false); // tries to set the handle(SOCKET) non-blocking
        
        if(version == IPVersion::IPv4){
            setSocketOptions(SocketOption::TCP_NoDelay, TRUE);
        }else{
            setSocketOptions(SocketOption::IPV6_Only, FALSE);
        }
    }

    
    Socket::Socket(const IPEndpoint& ipendpoint, const SOCKET& skt)
    :ipendpoint(ipendpoint), skt(skt){}   

    void Socket::setBlocking(const bool isBlocking){
        unsigned long blocking = 0, nonBlocking = 1;
        int res = ioctlsocket(skt, FIONBIO, isBlocking ? &blocking : &nonBlocking);

        if(res == SOCKET_ERROR){
            throw WSAGetLastError();
        }
    }

    void Socket::setSocketOptions(const SocketOption& option, BOOL value){
        int res = 0;
        if(option == SocketOption::TCP_NoDelay){
            res = setsockopt(skt, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
        }else{  
            res = setsockopt(skt, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
        }

        if(res == SOCKET_ERROR){
            throw WSAGetLastError();
        }
    }

    void Socket::_close(){
        if(skt == INVALID_SOCKET){
            throw Error::UninitializedSocket;
        }

        int res = closesocket(skt);

        if(res == INVALID_SOCKET){
            throw WSAGetLastError();
        }

        skt = INVALID_SOCKET;
    }

    void Socket::_bind(){
        const IPVersion version = ipendpoint.getIPVersion();
        
        int res = 0;
        if(version == IPVersion::IPv4){
            sockaddr_in addr = ipendpoint.getSockaddrIPv4();
            res = bind(skt, (sockaddr*)&addr, sizeof(sockaddr_in));
        }else{
            sockaddr_in6 addr = ipendpoint.getSockaddrIPv6();
            res = bind(skt, (sockaddr*)&addr, sizeof(sockaddr_in));
        }

        if(res == INVALID_SOCKET){
            throw WSAGetLastError();
        }
    }

    void Socket::_listen(){
        _bind(); // tries to bind the socket to ipendpoint
        int res = listen(skt, SOMAXCONN);

        if(res == SOCKET_ERROR){
            throw WSAGetLastError();
        }
    }

    void Socket::_accept(Socket& handle){
        const IPVersion version = this->ipendpoint.getIPVersion();
        
        IPEndpoint outipendpoint{};
        SOCKET outskt = INVALID_SOCKET;
        if(version == IPVersion::IPv4){
            sockaddr_in addr{};
            int len = sizeof(sockaddr_in);
            outskt = accept(this->skt, (sockaddr*)&addr, &len);

            if(outskt == INVALID_SOCKET){
                throw WSAGetLastError();
            }            

            outipendpoint = IPEndpoint((sockaddr*)&addr);
        }else{
            sockaddr_in6 addr{};
            int len = sizeof(sockaddr_in6);
            outskt = accept(this->skt, (sockaddr*)&addr, &len);

            if(outskt == INVALID_SOCKET){
                throw WSAGetLastError();
            }

            outipendpoint = IPEndpoint((sockaddr*)&addr);
        }
        handle = Socket(outipendpoint, outskt);
    }

    SOCKET Socket::getSocket(){
        return skt;
    }
  
    IPVersion Socket::getIPVersion(){
        return ipendpoint.getIPVersion();
    }

    void Socket::printSocketInfo(){
        const IPVersion version = ipendpoint.getIPVersion();
        const std::string ip_string = ipendpoint.getIP();
        const unsigned short port = ipendpoint.getPort();

        {
            std::cout << std::endl;
            if(version == IPVersion::IPv4){
                std::cout << "[Client-IP-VERSION] IPv4" << std::endl;
            }else{
                std::cout << "[Client-IP-VERSION] IPv6" << std::endl;
            }

            std::cout << "[Client-IP] " << ip_string << std::endl;
            std::cout << "[Client-PORT] " << port << std::endl;
            std::cout << std::endl;
        }

    }
}



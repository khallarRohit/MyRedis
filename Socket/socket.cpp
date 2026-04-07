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

/*

    connect() => The connect function establishes a connection to a specified socket.

    int WSAAPI connect(
        [in] SOCKET         s,
        [in] const sockaddr *name,
        [in] int            namelen
    );

    s => A descriptor identifying an unconnected socket.
    name => A pointer to the sockaddr structure to which the connection should be established.
    namelen => The length, in bytes, of the sockaddr structure pointed to by the name parameter.

    if no error => connect returns zero
    other wise SOCKET_ERROR

*/

namespace MyRedis{

    Socket::Socket(){};

    Socket::Socket(Socket&& handle) noexcept 
    : ipendpoint(handle.ipendpoint), skt(handle.skt), isBound(handle.isBound) {
        handle.skt = INVALID_SOCKET; 
        handle.isBound = false;
    }

    Socket::Socket(const IPEndpoint& ipendpoint)
    :ipendpoint(ipendpoint){
        IPVersion version = ipendpoint.getIPVersion();
        
        skt = socket((version == IPVersion::IPv4 ? AF_INET : AF_INET6), SOCK_STREAM, IPPROTO_TCP);
        if(skt == INVALID_SOCKET){
            throwWSAError("Socket/socket.cpp line:131");
        }
        
        setBlocking(false); // tries to set the handle(SOCKET) non-blocking
        
        if(version == IPVersion::IPv4){
            setSocketOptions(SocketOption::TCP_NoDelay, TRUE);
        }else{
            setSocketOptions(SocketOption::IPV6_Only, FALSE);
        }
    }

    
    Socket::Socket(const IPEndpoint& ipendpoint, SOCKET& skt) // expects socket to be non-blocking + options set
    :ipendpoint(ipendpoint), skt(skt){}   

    void Socket::setBlocking(const bool isBlocking){
        unsigned long blocking = 0, nonBlocking = 1;
        int res = ioctlsocket(skt, FIONBIO, isBlocking ? &blocking : &nonBlocking);

        if(res == SOCKET_ERROR){
            throwWSAError("Socket/socket.cpp line:152");
        }
    }

    Socket::Socket(const IPVersion& ipversion)
    :ipendpoint(ipversion){

        skt = socket((ipversion == IPVersion::IPv4 ? AF_INET : AF_INET6), SOCK_STREAM, IPPROTO_TCP);
        if(skt == INVALID_SOCKET){
            throwWSAError("Socket/socket.cpp line:161");
        }
        
        setBlocking(false); // tries to set the handle(SOCKET) non-blocking
        
        if(ipversion == IPVersion::IPv4){
            setSocketOptions(SocketOption::TCP_NoDelay, TRUE);
        }else{
            setSocketOptions(SocketOption::IPV6_Only, FALSE);
        }
    }

    Socket& Socket::operator=(Socket&& handle) noexcept {
        if (this != &handle) {
            if (skt != INVALID_SOCKET) {
                closesocket(skt); 
            }
            ipendpoint = handle.ipendpoint;
            skt = handle.skt;
            isBound = handle.isBound;
            
            handle.skt = INVALID_SOCKET; // Steal the resource
            handle.isBound = false;
        }
        return *this;
    }

    void Socket::setSocketOptions(const SocketOption& option, BOOL value){
        int res = 0;
        if(option == SocketOption::TCP_NoDelay){
            res = setsockopt(skt, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
        }else{  
            res = setsockopt(skt, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
        }

        if(res == SOCKET_ERROR){
            throwWSAError("Socket/socket.cpp line:188");
        }
    }

    void Socket::_close(){
        if(skt == INVALID_SOCKET){
            return;
        }

        int res = closesocket(skt);

        if(res == INVALID_SOCKET){
            throwWSAError("Socket/socket.cpp line:200");
        }

        skt = INVALID_SOCKET;
    }

    void Socket::_bind(){
        const IPVersion version = ipendpoint.getIPVersion();

        int res = 0;
        if(version == IPVersion::IPv4){
            sockaddr_in addr = ipendpoint.getSockaddrIPv4();
            res = bind(skt, (sockaddr*)(&addr), sizeof(sockaddr_in));
        }else{
            sockaddr_in6 addr = ipendpoint.getSockaddrIPv6();
            res = bind(skt, (sockaddr*)(&addr), sizeof(sockaddr_in6));
        }

        if(res == INVALID_SOCKET){
            throwWSAError("Socket/socket.cpp line:219");
        }

        isBound = true;
    }


    const bool Socket::checkBound(){
        return this->ipendpoint.getBound();
    };

    void Socket::_listen(){
        _bind(); // tries to bind the socket to ipendpoint
        int res = listen(skt, SOMAXCONN);

        if(res == SOCKET_ERROR){
            throwWSAError("Socket/socket.cpp line:235");
        }
    }

    const bool Socket::_accept(Socket& handle){

        const IPVersion version = this->ipendpoint.getIPVersion();
        
        IPEndpoint outipendpoint{IPVersion::unknown};
        SOCKET outskt = INVALID_SOCKET;
        if(version == IPVersion::IPv4){
            sockaddr_in addr{};
            int len = sizeof(sockaddr_in);
            outskt = accept(this->skt, (sockaddr*)(&addr), &len);

            if(outskt == INVALID_SOCKET){
                return false;
            }            

            outipendpoint = IPEndpoint((sockaddr*)&addr);
        }else{
            sockaddr_in6 addr{};
            int len = sizeof(sockaddr_in6);
            outskt = accept(this->skt, (sockaddr*)(&addr), &len);

            if(outskt == INVALID_SOCKET){
                return false;
            }

            outipendpoint = IPEndpoint((sockaddr*)&addr);
        }

        handle = Socket(outipendpoint, outskt);
        return true;
    }

    void Socket::_connect(const IPEndpoint& ipendpoint){
        if(this->getIPVersion() != ipendpoint.getIPVersion()){
            throw std::system_error(MyRedis::Error::InvalidIP, "Socket/socket.cpp line:273");
        }

        int res = 0;
        if(getIPVersion() == IPVersion::IPv4){
            sockaddr_in addr = ipendpoint.getSockaddrIPv4();
            res = connect(skt, (sockaddr*)(&addr), sizeof(sockaddr_in));
        }else{
            sockaddr_in6 addr = ipendpoint.getSockaddrIPv6();
            res = connect(skt, (sockaddr*)(&addr), sizeof(sockaddr_in6));
        }

        if(res != 0){
            int error = WSAGetLastError();

            if (error == WSAEWOULDBLOCK) { // trying to connect
                fd_set writeFds;
                FD_ZERO(&writeFds);
                FD_SET(skt, &writeFds);

                timeval timeout = { 2, 0 }; // Wait up to 2 seconds
                int selectRes = select(0, NULL, &writeFds, NULL, &timeout);     
                
                if (selectRes > 0) {
                    return; 
                } else if (selectRes == 0) {
                    throw std::system_error(std::error_code(WSAETIMEDOUT, std::system_category()), "Connection timed out");
                } else {
                    throwWSAError("Socket/socket.cpp line:311");
                }
            }
            else {
                throwWSAError("Socket/socket.cpp line:315");
            }
        }
    }

    Socket::~Socket(){
        _close();
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



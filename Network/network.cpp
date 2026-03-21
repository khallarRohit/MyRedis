#include "network.h"

/*
    The WSAStartup function initiates use of the Winsock DLL by a process.
    int WSAAPI WSAStartup(
        [in]  WORD      wVersionRequested,
        [out] LPWSADATA lpWSAData
    );

*/

namespace MyRedis{

    void Network::initialize(){
        WSAData wsadata;
        int res = WSAStartup(MAKEWORD(2, 2), &wsadata);
        if(res != 0){
            throwWSAError("Network/network.cpp line:18");
        }
    }

    void Network::shutdown(){
        int res = WSACleanup();
        if(res == SOCKET_ERROR){
            throwWSAError("Network/network.cpp line:25");
        }
    }

}
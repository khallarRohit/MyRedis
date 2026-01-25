#include "network.h"

/*
    The WSAStartup function initiates use of the Winsock DLL by a process.
    int WSAAPI WSAStartup(
        [in]  WORD      wVersionRequested,
        [out] LPWSADATA lpWSAData
    );

*/

namespace MyRedis{

    bool Network::initialize(){
        WSAData wsadata;
        int res = WSAStartup(MAKEWORD(2, 2), &wsadata);
        if(res != 0){
            throw WSAGetLastError();
        }
    }

    void Network::shutdown(){
        WSACleanup();
    }

}
#include "network.h"

/*
    The WSAStartup function initiates use of the Winsock DLL by a process.
    int WSAAPI WSAStartup(
        [in]  WORD      wVersionRequested,
        [out] LPWSADATA lpWSAData
    );

*/

namespace MyRedis{

    Network::Network() {
        WSAData wsadata;
        int res = WSAStartup(MAKEWORD(2, 2), &wsadata);
        if (res != 0) {
            throw std::system_error(res, std::system_category(), "WSAStartup failed");
        }
    }

    Network::~Network() {
        // Automatically called when the Network object goes out of scope
        int res = WSACleanup();
        if (res == SOCKET_ERROR) {
            std::cerr << "[!] Warning: WSACleanup failed during shutdown." << std::endl;
        }
    }

}
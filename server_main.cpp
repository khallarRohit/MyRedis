#include <iostream>
#include <csignal>
#include <atomic>
#include "Ip/ipendpoint.h"
#include "Server/server.h"

void print(const std::string& message){
    std::cout << "Error message: " << message << std::endl;
}

int main(){

    try{
        MyRedis::Network network;
        std::cout << "Winsock api Network initialized successfully." << std::endl;  

        MyRedis::Server server(MyRedis::IPEndpoint("127.0.0.1", 6112));
        server.initialize();  // tries to initialize a server
        server.printServerInfo();

        int failCount = server.getPollFailCount();
        bool listeningSocketFailed = false;
        while(true){
            server.frame(failCount, listeningSocketFailed);
            if(failCount == 0 or listeningSocketFailed) break;
            // std::cout << "server working ..." << std::endl;
        }

        if(failCount == 0){
            std::cerr << "[!] Error: System out of buffers." << std::endl;
        }

    }catch(const std::system_error& error){
        std::cerr << "[!] code:" << error.code() << " " << error.what() << std::endl;
    }

    return 0;
}
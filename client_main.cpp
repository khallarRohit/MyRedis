#include <iostream>
#include "Client/client.h"
#include "Ip/ipendpoint.h"


int main(){

    try{
        MyRedis::Network::initialize(); // tries to initialize the network
        std::cout << "Winsock api Network initialized successfully." << std::endl;  
        
        Client client(MyRedis::IPVersion::IPv4);
        client.connect(MyRedis::IPEndpoint("127.0.0.1", 6112));


    }catch(const std::system_error& error){
        std::cerr << "[!] code:" << error.code() << " " << error.what() << std::endl;
    }

    try{
        MyRedis::Network::shutdown();
        std::cout << "Winsock api Network shutdown successfully." << std::endl;

    }catch(const std::system_error& error){
        std::cerr << "[!] code:" << error.code() << " " << error.what() << std::endl;
    }

}
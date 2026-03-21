#include "client.h"

Client::Client(const MyRedis::IPVersion& ipversion)
:socket(ipversion){}

void Client::connect(const MyRedis::IPEndpoint& ipendpoint){
    if(connected){
        return;
    }

    socket._connect(ipendpoint);
    connected = true;
    std::cout << "Socket connected Successfully." << std::endl;    
}


const bool Client::isConnected(){
    return connected;
}


void Client::Frame(){

}


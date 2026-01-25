#include "Server/server.h"

void print(const std::string& message){
    std::cout << "Error message: " << message << std::endl;
}

int main() {
    
    MyRedis::Server server(MyRedis::IPEndpoint("127.0.0.1", 4790));
    try{
        server.initialize(); 
    }catch(...){
        std::cerr << "some error occured" << std::endl;
    }

    while(true){
        try{
            server.frame();
        }catch(int error){
            switch(error){
                case WSAENETDOWN:
                    print("The network subsystem has failed.");
                    break;
                case WSAEFAULT:
                    print("An exception occurred while reading user input parameters.");
                    break;
                case WSAEINVAL:
                    print("An invalid parameter was passed.");
                    break;
                case WSAENOBUFS:
                    print("Some function was unable to allocate sufficient memory.");
                    break;
                default:
                    break;
            }
        }catch(...){
            print("Some unknown error occured.");
        }
    }

    MyRedis::Network::shutdown();
    system("pause");
    return 0;
}
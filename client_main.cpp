#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <WinSock2.h>
#include "Network/network.h"
#include "Ip/ipendpoint.h"
#include "Socket/socket.h"

std::string encodeToRESP(const std::string& input) {
    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string word;
    
    while (iss >> word) {
        args.push_back(word);
    }
    
    if (args.empty()) return "";

    std::string resp = "*" + std::to_string(args.size()) + "\r\n";
    for (const auto& arg : args) {
        resp += "$" + std::to_string(arg.length()) + "\r\n" + arg + "\r\n";
    }
    
    return resp;
}

int main() {
    try {
        MyRedis::Network network; 

        MyRedis::IPEndpoint serverIP("127.0.0.1", 6112);
        MyRedis::Socket clientSocket(MyRedis::IPVersion::IPv4);
        
        std::cout << "Connecting to MyRedis Server at 127.0.0.1:6112..." << std::endl;
        
        clientSocket._connect(serverIP);
        std::cout << "Connected successfully! Type 'exit' to quit.\n" << std::endl;

        while (true) {
            std::string input;
            std::cout << "127.0.0.1:6112> ";
            
            if (!std::getline(std::cin, input) || input == "exit" || input == "quit") {
                break;
            }
            if (input.empty()) continue;

            std::string command = encodeToRESP(input);
            if (command.empty()) continue;
            
            int bytesSent = send(clientSocket.getSocket(), command.c_str(), command.length(), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "[-] Failed to send data." << std::endl;
                break;
            }

            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(clientSocket.getSocket(), &readfds);
            
            timeval timeout{5, 0};
            int res = select(0, &readfds, NULL, NULL, &timeout);
            
            if (res > 0) {
                char buffer[4096];
                int bytesReceived = recv(clientSocket.getSocket(), buffer, sizeof(buffer) - 1, 0);
                
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0';
                    std::cout << buffer; 
                } else if (bytesReceived == 0) {
                    std::cout << "[-] Server closed the connection." << std::endl;
                    break;
                }
            } else if (res == 0) {
                std::cout << "[-] (Timeout waiting for server response)" << std::endl;
            } else {
                std::cerr << "[-] Select error occurred." << std::endl;
                break;
            }
        }
    } catch(const std::exception& e) {
        std::cerr << "[!] Error: " << e.what() << std::endl;
    }

    return 0;
}
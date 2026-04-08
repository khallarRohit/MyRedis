#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <WinSock2.h>
#include "Network/network.h"
#include "Ip/ipendpoint.h"
#include "Socket/socket.h"

// --- NEW HELPER FUNCTION ---
// Translates raw text like "SET name John" into "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$4\r\nJohn\r\n"
std::string encodeToRESP(const std::string& input) {
    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string word;
    
    // Split the input string by spaces
    while (iss >> word) {
        args.push_back(word);
    }
    
    if (args.empty()) return "";

    // Build the RESP Array
    std::string resp = "*" + std::to_string(args.size()) + "\r\n";
    for (const auto& arg : args) {
        resp += "$" + std::to_string(arg.length()) + "\r\n" + arg + "\r\n";
    }
    
    return resp;
}

int main() {
    try {
        // 1. Initialize Winsock using your brilliant RAII wrapper
        MyRedis::Network network; 

        // 2. Setup the endpoint and socket
        MyRedis::IPEndpoint serverIP("127.0.0.1", 6112);
        MyRedis::Socket clientSocket(MyRedis::IPVersion::IPv4);
        
        std::cout << "Connecting to MyRedis Server at 127.0.0.1:6112..." << std::endl;
        
        // 3. Connect (your _connect method handles the non-blocking handshake!)
        clientSocket._connect(serverIP);
        std::cout << "Connected successfully! Type 'exit' to quit.\n" << std::endl;

        // 4. The REPL Loop (Read, Eval, Print, Loop)
        while (true) {
            std::string input;
            std::cout << "127.0.0.1:6112> ";
            
            // Get user input from the console
            if (!std::getline(std::cin, input) || input == "exit" || input == "quit") {
                break;
            }
            if (input.empty()) continue;

            // --- UPDATED LOGIC: Use the RESP encoder ---
            std::string command = encodeToRESP(input);
            if (command.empty()) continue;
            
            // Send the command
            int bytesSent = send(clientSocket.getSocket(), command.c_str(), command.length(), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "[-] Failed to send data." << std::endl;
                break;
            }

            // Because your Socket class forces non-blocking mode, we must wait 
            // for the server to reply using select() before we call recv()
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(clientSocket.getSocket(), &readfds);
            
            timeval timeout{5, 0}; // Wait up to 5 seconds for the server to reply
            int res = select(0, &readfds, NULL, NULL, &timeout);
            
            if (res > 0) {
                char buffer[4096];
                int bytesReceived = recv(clientSocket.getSocket(), buffer, sizeof(buffer) - 1, 0);
                
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0'; // Null-terminate the raw bytes
                    std::cout << buffer; // Print the server's reply
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
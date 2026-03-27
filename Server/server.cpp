#include "server.h"

/*
    WSAPOLLFD struct store information used by WSAPoll function
    typedef struct pollfd {
        SOCKET fd;
        SHORT  events;
        SHORT  revents;
    } WSAPOLLFD, *PWSAPOLLFD, *LPWSAPOLLFD;

    fd(SOCKET) => The identifier of the socket for which to find status
    events(short) => A set of flags indicating the type of status being requested.

    POLLRDNORM => Normal data can be read without blocking.
    POLLWRNORM => Normal data can be written without blocking.

    revents(short) => A set of flags that indicate, upon return from the WSAPoll function call, the results of the status query.   

*/


/*
    WSAPoll function determine status of one or more sockets

    int WSAAPI WSAPoll(
        [in, out] LPWSAPOLLFD fdArray,
        [in]      ULONG       fds,
        [in]      INT         timeout
    );

    fdArray => An array of one or more POLLFD structures specifying the set of 
    sockets for which status is requested. The array must contain at least one 
    structure with a valid socket. Upon return, this parameter receives the 
    updated sockets with the revents status flags member set on each one that
    matches the status query criteria.

    fds => no of poll structure in fd array

    timeout => wait time for some request to hit

*/


/*
    The recv function receives data from a connected socket or a bound connectionless socket.

    int WSAAPI recv(
        [in]  SOCKET s,
        [out] char   *buf,
        [in]  int    len,
        [in]  int    flags
    );

    s => socket descriptor that identifies a connected socket
    buf => pointer to buffer to receive incomming data
    len => The length, in bytes, of the buffer pointed to by the buf parameter.
    flags => A set of flags that influences the behavior of this function.

    If no error occurs, recv returns the number of bytes received and the buffer pointed to by the buf parameter will contain this data received.

    in case of error a value of SOCKET_ERROR is returned
*/

namespace MyRedis{

    
    Server::Server(const IPEndpoint&& ipendpoint)
    :listeningSocket(ipendpoint){
        ctx = std::make_shared<SharedLock>();
        std::cout << "Listening Socket created successfully" << std::endl;
    }

    void Server::initialize(){
        fdList.clear();
        connections.clear();      

        listeningSocket._listen(); // tries to bind socket to endpoint + listen on the endpoint
        std::cout << "Socket is listening" << std::endl;

        WSAPOLLFD listeningSocketfd{};
        listeningSocketfd.fd = listeningSocket.getSocket();
        listeningSocketfd.events = POLLRDNORM; // reading event
        listeningSocketfd.revents = 0;

        fdList.push_back(listeningSocketfd);
    }

    void Server::frame(int& failCount, bool& listeningSocketFailed){
        std::vector<WSAPOLLFD> tempfdList = fdList;

        int res = WSAPoll(tempfdList.data(), tempfdList.size(), 1000);
        if(res == 0) { // no sockets were in the queried state before the timer expired.
            return;
        }else if(res == SOCKET_ERROR){ // error occured
            if(WSAGetLastError() == WSAENOBUFS){
                failCount--;
                std::cerr << "[!] Warning: System running out of buffers." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return;
            }

            failCount = pollFailCount;
            throwWSAError("Server/server.cpp line:102");
        }

        WSAPOLLFD& listeningSocketfd = tempfdList[0];

        if(listeningSocketfd.revents & POLLNVAL){
            listeningSocketFailed = true;
            std::cerr << "[!] Error: Invalid ListeningSocket." << std::endl; 
            return;
        }
        
        if(listeningSocketfd.revents & POLLERR){
            listeningSocketFailed = true;
            std::cerr << "[!] Error: ListeningSocket error." << std::endl;
            return;
        }

        if(listeningSocketfd.revents & POLLRDNORM){ // enter only if some request has come to listening socket
            
            Socket newConnectionSocket{};
            const bool res = listeningSocket._accept(newConnectionSocket);

            if(res){
                connections.emplace_back(TCPConnection(newConnectionSocket, ctx));
                std::cout << "~ New Connection Accepted" << std::endl;
                TCPConnection& newTCPConnection = connections.back();
                newTCPConnection.printClientInfo();

                WSAPOLLFD newConnectionFd{};
                newConnectionFd.fd = newTCPConnection.socket->getSocket();
                newConnectionFd.events = POLLRDNORM | POLLWRNORM; // read + write event allowed
                newConnectionFd.revents = 0;

                fdList.push_back(newConnectionFd);
            }else{
                std::cerr << "~ " << getWSAMessage(WSAGetLastError()) << std::endl;
            }
        }

        for(int i=tempfdList.size()-1;i>=1;i--){
            int connectionIndex = i-1;
            TCPConnection& TCPConnection = connections[connectionIndex];
            WSAPOLLFD& connectionfd = tempfdList[i];

            if(connectionfd.revents & POLLERR){
                closeConnection(connectionIndex, "POLLERR");
                continue;
            }
            if(connectionfd.revents & POLLHUP){
                closeConnection(connectionIndex, "POLLHUP");
                continue;
            }
            if(connectionfd.revents & POLLNVAL){
                closeConnection(connectionIndex, "POLLNVAL");
                continue;
            }    
            
            if(connectionfd.revents & POLLRDNORM){ // normal data can be read without blocking 
                std::shared_ptr<Packet> packet = TCPConnection.packetManager->getPacket();
                int bytesReceived = 0;
                
                if(packet == nullptr){
                    bytesReceived = recv(connectionfd.fd,
                        (char*)&TCPConnection.packetManager->queryType + TCPConnection.packetManager->extractionOffSet,
                        sizeof(int32_t) - TCPConnection.packetManager->extractionOffSet, 0); 
                    TCPConnection.packetManager->extractionOffSet += bytesReceived;

                    if(TCPConnection.packetManager->extractionOffSet == sizeof(int32_t)){
                        TCPConnection.packetManager->queryType = ntohs(TCPConnection.packetManager->queryType);
                        if(TCPConnection.packetManager->queryType == 0){
                            TCPConnection.packetManager->createPacket(GET);
                        }else{
                            TCPConnection.packetManager->createPacket(POST);
                        }
                    }
                    continue;
                }

                char* targetBuffer = packet->packetQuery->getCurrentTargetBuffer();
                int32_t targetSpaceLeft = packet->packetQuery->getRemainingBufferSize();

                bytesReceived = recv(connectionfd.fd, targetBuffer, targetSpaceLeft, 0);
                packet->resolveTask(bytesReceived);

                if(packet->getState() == FULL){
                    TCPConnection.packetManager->processPacket();
                }    
            }
            
            if(connectionfd.revents & POLLWRNORM){ // normal data can be written without blocking 
                if(TCPConnection.packetManager->hasDataToSend()){
                    const char* targetBuffer = TCPConnection.packetManager->getCurrentWriteBuffer();
                    int32_t targetSpaceLeft = TCPConnection.packetManager->getWriteRemainingSize();

                    int bytesSent = send(connectionfd.fd, targetBuffer, targetSpaceLeft, 0);

                    if(bytesSent == SOCKET_ERROR){
                        if(WSAGetLastError() != WSAEWOULDBLOCK) {
                            closeConnection(connectionIndex, "Send Error");
                            continue;
                        }              
                    }else if(bytesSent > 0){
                        TCPConnection.packetManager->resolveWrite(bytesSent);
                    }                    
                }

                // if(!tcpConnection.manager->hasDataToSend()) {
                //     fdList[i].events &= ~POLLWRNORM; 
                // }
            }
        }
        
    }


    void Server::closeConnection(int connectionIndex, std::string&& reason){
        TCPConnection& connection = connections[connectionIndex];

        std::cout << "[" << reason << "] Connection lost: " << std::endl;
        connection.printClientInfo();

        fdList.erase(fdList.begin() + (connectionIndex + 1));
        connections.erase(connections.begin() + connectionIndex);
    }  

    void Server::printServerInfo(){
        listeningSocket.printSocketInfo();
    }

    int Server::getPollFailCount(){
        return pollFailCount;
    }

}





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

namespace MyRedis{
    
    Server::Server(const IPEndpoint&& ipendpoint)
    :listeningSocket(ipendpoint){
        std::cout << "Listening Socket created successfully" << std::endl;
    };

    void Server::initialize(){
        fdList.clear();
        connections.clear();

        Network::initialize(); // tries to initialize the network
        std::cout << "Winsock api Network initialized successfully." << std::endl;        

        listeningSocket._listen(); // tries to bind socket to endpoint + listen on the endpoint
        std::cout << "Socket is listening" << std::endl;

        WSAPOLLFD listeningSocketfd{};
        listeningSocketfd.fd = listeningSocket.getSocket();
        listeningSocketfd.events = POLLRDNORM; // reading event
        listeningSocketfd.revents = 0;

        fdList.push_back(listeningSocketfd);
    }

    void Server::frame(){
        std::vector<WSAPOLLFD> tempfdList = fdList;

        int res = WSAPoll(tempfdList.data(), tempfdList.size(), -11);
        if(res == 0) { // no sockets were in the queried state before the timer expired.
            return;
        }else if(res == SOCKET_ERROR){ // error occured
            throw WSAGetLastError();
        }

        WSAPOLLFD& listeningfd = tempfdList[0];
        if(listeningfd.revents & POLLRDNORM){ // enter only if some request has come to listening socket
            
            Socket newConnection{};
            TCPConnection newtcpConnection(newConnection);
            connections.push_back(newtcpConnection);
            std::cout << "New Connection Accepted" << std::endl;
            newtcpConnection.printClientInfo();

            WSAPOLLFD acceptedConnectionfd{};
            acceptedConnectionfd.fd = newtcpConnection.handle.getSocket();
            acceptedConnectionfd.events = POLLRDNORM | POLLWRNORM;
            acceptedConnectionfd.revents = 0;

            fdList.push_back(acceptedConnectionfd);
        }

        for(int i=tempfdList.size()-1;i>=0;i--){
            int connectionIndex = i-1;
            TCPConnection& connection = connections[connectionIndex];
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
 
            if(tempfdList[i].revents & POLLRDNORM){ // this client is ready to read normal data without blocking
                


            }

        }



    }

    void Server::closeConnection(int connectionIndex, std::string&& reason){
        TCPConnection& connection = connections[connectionIndex];
        std::cout << "[" << reason << "] Connection lost: " << std::endl;
        connection.printClientInfo();
        fdList.erase(fdList.begin() + (connectionIndex + 1));
        connection._close();
        connections.erase(connections.begin() + connectionIndex);
    }   
}





#ifndef __CONNECTION_MANAGER_H__
#define __CONNECTION_MANAGER_H__

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <poll.h>

// #include "TCPSocket.hpp"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#define	TCP_NO_ERROR		    0  // no error
#define	TCP_SOCKET_ERROR	    1  // invalid socket
#define	TCP_ADDRESS_ERROR	    2  // invalid port and/or IP address
#define	TCP_SOCKOP_ERROR	    3  // socket operator (socket, listen, bind, accept,...) error
#define TCP_CONNECTION_CLOSED	4  // send/receive indicate connection is closed
#define	TCP_MEMORY_ERROR	    5  // mem alloc error

#define TIMEOUT         100 // in seconds
#define MAX_PENDING     10  // max connections

#define FAILURE         -1  // return status
#define SUCCESS         0   // return status

typedef struct tcp_socket {
    int id;
    int sd;
    int port;
    string ipAddress;
} tcp_socket_t;

class ConnectionManager {
    private:
        int serverListeningPort;
        char serverIPAddress[15];
        pthread_t threadID; 
    public:
        ConnectionManager(int port);
        void displayIPAdrress();
        void displayPortNumber();
        int connectToDestination(string& destinationIP, int port);
        void displayAllActiveConnection();
        bool terminateConnection(int connectionID);
        bool senDataToConnection(int connectionID);
        int tcpOpen(tcp_socket_t** socket, int port);
        int onStart();
};

#ifdef __cplusplus
}
#endif

#endif // __CONNECTION_MANAGER_H__
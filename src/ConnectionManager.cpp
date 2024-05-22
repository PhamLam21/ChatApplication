#include "../inc/ConnectionManager.hpp"

static struct pollfd *poll_fds = NULL;
static tcp_socket_t serverConnection;
static vector<tcp_socket_t> activeConnections;
static int serverFd;

ConnectionManager::ConnectionManager(int port): serverListeningPort(port)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    memcpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

    /*Accessing network interface information by passing address using ioctl.*/
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    /*Extract IP Address*/
    strcpy(serverIPAddress, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
    cout << "System IP Address is: " << serverIPAddress << endl;
    cout << "Listening at port: " << port << endl;
}

void ConnectionManager::displayIPAdrress()
{
    cout << "System IP Address is: " << serverIPAddress << endl;
}

void ConnectionManager::displayPortNumber()
{
    cout << "Listening port is: " << serverListeningPort << endl;
}

int ConnectionManager::connectToDestination(string& destinationIP, int port)
{
    tcp_socket_t serverConnection;
    struct sockaddr_in serverAddr;

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        cerr << "Failed to create a socket" << endl;
        return -1;
    }

    // Prepare the server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(destinationIP.c_str());

    // Attempt to connect to the specified destination
    if (connect(serverFd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        cerr << "Failed to connect to the destination" << endl;
        close(serverFd);
        return false;
    }

    serverConnection.id = activeConnections.size() + 1;
    serverConnection.sd = serverFd;
    serverConnection.port = port;
    serverConnection.ipAddress = destinationIP;
    activeConnections.push_back(serverConnection);

    cout << "Connected to " << destinationIP << " on port " << port << "(" << serverFd << ")" << endl;

    return SUCCESS;
}

void ConnectionManager::displayAllActiveConnection()
{
    cout << "List all active connections" << endl;
    if (activeConnections.empty()) {
        cout << "No active connections." << endl;
    } else {
        cout << "id\tIP adrress\tPort No." << endl;
        for (size_t i = 0; i < activeConnections.size(); ++i) {
            const tcp_socket_t& conn = activeConnections[i];
            cout << i + 1 << "\t" << conn.ipAddress << "\t" << conn.port << endl;
        }
    }
    
}

void ConnectionManager::addConnection(const string& ipAddress, int port)
{
    tcp_socket_t newConnection;

    newConnection.id = activeConnections.size() + 1;
    newConnection.sd = -1;
    newConnection.port = port;
    newConnection.ipAddress = ipAddress;
    activeConnections.push_back(newConnection);
}

void reorganizeConnectionIds(std::vector<tcp_socket_t>& connections) {
    int newId = 1;
    for (tcp_socket_t& conn : connections) {
        conn.id = newId++;
    }
}

bool ConnectionManager::terminateConnection(int connectionID)
{
    bool isConnectionIDValid = false;

    for (auto conn = activeConnections.begin(); conn != activeConnections.end();) {
        if (conn->id == connectionID) {
            conn = activeConnections.erase(conn);
            isConnectionIDValid = true;
        } else {
            ++conn;
        }
    }

    reorganizeConnectionIds(activeConnections);
    displayAllActiveConnection();

    return isConnectionIDValid;
}

bool ConnectionManager::senDataToConnection(int connectionID)
{
    char choice;
    int numbWrited;
    bool continueLoop = true;

    for (auto conn = activeConnections.begin(); conn != activeConnections.end();) {
        if (conn->id == connectionID) {
            while (continueLoop) {
                string sendBuffer = "";

                // Clear the input buffer
                cout << "Please enter the messagee: ";
                cin.ignore();
                getline(cin, sendBuffer, '\n');

                numbWrited = write(conn->sd, sendBuffer.c_str(), sizeof(sendBuffer));
                if (numbWrited == FAILURE)     
                    cout << "connectToDestination: failured write to server" << endl;

                cout << "Do you want to continue (y/n)? ";
                cin >> choice;

                switch (choice) {
                    case 'y':
                        cout << "Continuing..." << endl;
                        break;
                    
                    case 'n':
                        cout << "Exiting..." << endl;
                        continueLoop = false;
                        return SUCCESS;
                    
                    default:
                        cout << "Invalid choice. Please enter 'y' or 'n'." << endl;
                        break;
                }
            }
        } else {
            ++conn;
        }
    }

    close(serverFd);

    return SUCCESS;
}

int ConnectionManager::tcpOpen(tcp_socket_t** s, int port)
{
    int result, opt;
    struct sockaddr_in addr;
    tcp_socket_t *tmpSocket;

    tmpSocket = (tcp_socket_t *)malloc(sizeof(tcp_socket_t));

    tmpSocket->sd = socket(AF_INET, SOCK_STREAM, 0);
    if (tmpSocket->sd == FAILURE)
        cerr << "Create socket failure" << endl;

    if (setsockopt(tmpSocket->sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        cerr << "setsockopt failure" << endl;

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(tmpSocket->sd, (struct sockaddr*)&addr, sizeof(addr)) == FAILURE)
        perror("bind");

    if (listen(tmpSocket->sd, MAX_PENDING) == -1)
        cerr << "Listening socket failure" << endl;
 
    *s = tmpSocket;
    return TCP_NO_ERROR;
}

void ConnectionManager::tcpClose(tcp_socket_t** socket, int port)
{

}

static int tcpWaitForConnection(tcp_socket_t* socket, tcp_socket_t** newSocket)
{
    int result;
    struct sockaddr_in addr;
    tcp_socket_t *tmpSocket;
    unsigned int length = sizeof(struct sockaddr_in);
    char * p;

    tmpSocket = (tcp_socket_t *)malloc(sizeof(tcp_socket_t));
    tmpSocket->sd = accept(socket->sd, (struct sockaddr*) &addr, &length);

    p = inet_ntoa(addr.sin_addr);

    tmpSocket->port = ntohs(addr.sin_port);
    tmpSocket->ipAddress.append(p);

    *newSocket = tmpSocket;
    return TCP_NO_ERROR;
}

void ConnectionManager::tcpSend(tcp_socket_t* socket, void* buffer, int* size )
{

}
void ConnectionManager::tcpRecevice(tcp_socket_t* socket, void* buffer, int* size )
{

}
void ConnectionManager::tcpGetIPAddress(tcp_socket_t* socket, string& ipAddress)
{

}
void ConnectionManager::tcpGetPort(tcp_socket_t* socket, int* port)
{

}
void ConnectionManager::tcpGetSd(tcp_socket_t* socket, int* sd)
{

}

static void *connectionManagerThread(void *arg)
{   
    int success = FAILURE;
    int poll_res;
    int connCounter = 0;
    int numbRead;
    tcp_socket_t* sPointer = NULL;
    tcp_socket_t dumpy;

    poll_fds = (struct pollfd *)malloc(sizeof(struct pollfd));
    poll_fds[0].fd = serverConnection.sd;
    poll_fds[0].events = POLLIN;

    while ((poll_res = poll(poll_fds, (connCounter+1), TIMEOUT*1000)) || connCounter) {
        if (poll_res == FAILURE)
            break;

        // When an event is received from Server socket, create new socket unless limit is reached
        if ((poll_fds[0].revents & POLLIN) && connCounter < MAX_PENDING) {
            
            // Blocks until a connection is processed
            if((tcpWaitForConnection(&serverConnection, &(sPointer))) != TCP_NO_ERROR) {
                cerr << time(NULL) << " connectionManagerThread: failed to accept new connection (" << success << ")" << endl; 
            }

            // cout << "tmpSocket->sd : " << sPointer->sd  << endl;
            // cout << "Port: " << sPointer->port << endl;
            // cout << "IP: " << sPointer->ipAddress << endl;

            connCounter++; // Increment number of connections
            poll_fds = (struct pollfd *)realloc(poll_fds, sizeof(struct pollfd)*(connCounter+1)); // Increase poll_fd array size
            poll_fds[connCounter].fd = sPointer->sd;
            poll_fds[connCounter].events = POLLIN | POLLHUP;

            // Push to list active connection
            activeConnections.push_back(*sPointer);
            poll_res--;
        }

        // When an event is received from Client socket
        for (int i = 1; i < (connCounter+1) && poll_res > 0; i++) {
            if((poll_fds[i].revents & POLLIN)) {
                char reciveBuffer[100];
                memset(reciveBuffer, '0', sizeof(reciveBuffer));
                numbRead = read(poll_fds[i].fd, reciveBuffer, sizeof(reciveBuffer));
                if (numbRead == -1)
                    cout << "connectionManagerThread: read failed" << endl;
                else {
                    for (auto conn = activeConnections.begin(); conn != activeConnections.end();) {
                        if (conn->sd == poll_fds[i].fd) {
                            cout << "\nMessage received from " << conn->ipAddress << endl;
                            cout << "Sender’s Port: <" << ntohs(conn->port) << ">" << endl;
                            break;
                        } else {
                            ++conn;
                        }
                    }
                    cout << "Message: " << reciveBuffer << endl;
                    break;
                }
            }
        }
    }
}

int ConnectionManager::onStart()
{
    tcp_socket_t *sPointer = NULL;

    // Initially array for 1 element (listening server)
    if (tcpOpen(&sPointer, serverListeningPort) != TCP_NO_ERROR) {
        cerr << "ConnectionManager: failed to start" << endl;
        return FAILURE;
    }

    serverConnection.id = activeConnections.size() + 1;
    serverConnection.sd = sPointer->sd;
    serverConnection.port = serverListeningPort;
    serverConnection.ipAddress = serverIPAddress;
    activeConnections.push_back(serverConnection);

    if (pthread_create(&threadID, NULL, &connectionManagerThread, NULL)) {
        cerr << "ConnectionManager: " << __func__ << " " << __LINE__ << endl;
        return -1;
    }

    cout << "ConnectionManager: start successfully" << endl;
}
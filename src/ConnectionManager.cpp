#include "../inc/ConnectionManager.hpp"
#include "ConnectionManager.hpp"

static struct pollfd *poll_fds = NULL;
static vector<tcp_socket_t> activeConnections;
static int serverFd;

ConnectionManager::ConnectionManager(int port): serverListeningPort(port)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
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

static int tcpWaitForConnection(tcp_socket_t *socket, tcp_socket_t **newSocket)
{
    int result;
    struct sockaddr_in addr;
    tcp_socket_t *tmpSocket;
    unsigned int length = sizeof(struct sockaddr_in);
    char * p;

    tmpSocket = (tcp_socket_t *)malloc(sizeof(tcp_socket_t));
    tmpSocket->sd = accept(socket->sd, (struct sockaddr*) &addr, &length);

    p = inet_ntoa(addr.sin_addr);
    tmpSocket->id = activeConnections.size();
    tmpSocket->port = ntohs(addr.sin_port);
    tmpSocket->ipAddress.append(p);

    *newSocket = tmpSocket;
    return TCP_NO_ERROR;
}
void reorganizeConnectionIds(std::vector<tcp_socket_t>& connections) {
    int newId = 0;
    for (tcp_socket_t& conn : connections) {
        conn.id = newId++;
    }
}
static void* connectionManagerThread(void *arg)
{   
    int success = FAILURE;
    int poll_res;
    int connCounter = 0;
    int numbRead;
    tcp_socket_t* sPointer = (tcp_socket_t*) arg;
    tcp_socket_t* tmpSocket = NULL;
    tcp_socket_t dumpy;

    poll_fds = (struct pollfd *)malloc(sizeof(struct pollfd));
    poll_fds[0].fd = sPointer->sd;
    poll_fds[0].events = POLLIN;

    while ((poll_res = poll(poll_fds, (connCounter+1), TIMEOUT*1000)) || connCounter) {
        if (poll_res == FAILURE)
            break;

        // When an event is received from Server socket, create new socket unless limit is reached
        if ((poll_fds[0].revents & POLLIN) && connCounter < MAX_PENDING) {
            
            // Blocks until a connection is processed
            if((tcpWaitForConnection(sPointer, &(tmpSocket))) != TCP_NO_ERROR) {
                cerr << time(NULL) << " connectionManagerThread: failed to accept new connection (" << success << ")" << endl; 
            }

            cout << "tmpSocket->sd : " << tmpSocket->sd  << endl;
            cout << "Port: " << tmpSocket->port << endl;
            cout << "IP: " << tmpSocket->ipAddress << endl;

            connCounter++; // Increment number of connections
            poll_fds = (struct pollfd *)realloc(poll_fds, sizeof(struct pollfd)*(connCounter+1)); // Increase poll_fd array size
            poll_fds[connCounter].fd = tmpSocket->sd;
            poll_fds[connCounter].events = POLLIN | POLLHUP;

            // Push to list active connection
            activeConnections.push_back(*tmpSocket);
            poll_res--;
        }

        // When an event is received from Client socket
        for (int i = 1; i < (connCounter+1) && poll_res > 0; i++) {
            if((poll_fds[i].revents & POLLIN)) {
                char buf[100];
                memset(buf, '0', sizeof(buf));
                numbRead = read(poll_fds[i].fd, buf, sizeof(buf));
                if (numbRead == -1)
                    cout << "connectionManagerThread: read failed" << endl;
                else {
                    buf[numbRead] = 0;
                    for (auto conn = activeConnections.begin(); conn != activeConnections.end();) {
                        if (conn->sd == poll_fds[i].fd) {
                            if((strncmp(buf, "exit", 4) == 0) || (numbRead == 0))
                            {
                                memset(buf, '0', sizeof(buf));
                                strncpy(buf, "exit", 4);
                                conn = activeConnections.erase(conn);
                                close(conn->sd);
                                reorganizeConnectionIds(activeConnections);
                            }
                            cout << "\nMessage received from " << conn->ipAddress << endl;
                            cout << "Sender’s Port: <" << ntohs(conn->port) << ">" << endl;
                            cout << "Message: " << buf << endl;
                            break;
                        }
                        else {
                            ++conn;
                        }
                    }    
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

    sPointer->id = 0;
    sPointer->port = serverListeningPort;
    sPointer->ipAddress = serverIPAddress;
    activeConnections.push_back(*sPointer);

    if (pthread_create(&threadID, NULL, &connectionManagerThread, sPointer)) {
        cerr << "ConnectionManager: " << __func__ << " " << __LINE__ << endl;
        return -1;
    }
    pthread_detach(threadID);
    cout << "ConnectionManager: start successfully" << endl;

    return SUCCESS;
}
void ConnectionManager::displayIPAdrress()
{
    cout << "System IP Address is: " << serverIPAddress << endl;
}

void ConnectionManager::displayPortNumber()
{
    cout << "Listening port is: " << serverListeningPort << endl;
}
static void* recvDataThread(void *arg)
{
    
    int client = *(int *)arg;
    auto tmp = activeConnections.begin();
    for (auto conn = activeConnections.begin(); conn != activeConnections.end();) {
        if (conn->sd == client) {
            tmp = conn;
            break;
        } else {
            ++conn;
        }
    }
    char buf[256];
    while (1)
    {
        int numberRead = recv(client, buf, sizeof(buf), 0);
        if((strncmp(buf, "exit", 4) == 0) || (numberRead == 0))
        {
            memset(buf, '0', sizeof(buf));
            strncpy(buf, "exit", 4);
            break;
        }
        buf[numberRead] = 0;
        cout << "\nMessage received from " << tmp->ipAddress << endl;
        cout << "Sender’s Port: <" << tmp->port << ">" << endl;
        cout << "Message: " << buf << endl;
    }
    tmp = activeConnections.erase(tmp);
    close(tmp->sd);
    reorganizeConnectionIds(activeConnections);
    return 0;
}
int ConnectionManager::connectToDestination(string& destinationIP, int port)
{
    tcp_socket_t newConnection;
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
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, recvDataThread, (void *)&serverFd))
    {
        printf("Khong the tao luong!\n");
        return 1;   
    }
    // Chuyển luồng sang chế độ tự giải phóng
    pthread_detach(thread_id);

    newConnection.id = activeConnections.size();
    newConnection.sd = serverFd;
    newConnection.port = port;
    newConnection.ipAddress = destinationIP;
    activeConnections.push_back(newConnection);

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
            cout << conn.id << "\t" << conn.ipAddress << "\t" << conn.port << endl;
        }
    }
    
}

bool ConnectionManager::terminateConnection(int connectionID)
{
    bool isConnectionIDValid = false;
    for (auto conn = activeConnections.begin(); conn != activeConnections.end();) {
        if (conn->id == connectionID) {
            write(conn->sd, "exit", 4);
            conn = activeConnections.erase(conn);
            close(conn->sd);
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



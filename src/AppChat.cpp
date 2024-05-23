#include "../inc/AppChat.hpp"

AppChat* AppChat::_instance = NULL;
ConnectionManager* AppChat::connmgr = NULL;

AppChat* AppChat::getInstance(int portno)
{
    if (_instance == NULL) {
        _instance = new AppChat();
    }
    
    connmgr = new ConnectionManager(portno);
    connmgr->onStart();

    return _instance;
}

AppChat::AppChat() {}

void AppChat::displayMenu()
{
    cout << "\n\n#################################################" << endl;
    cout << "Select these functions below" << endl;
    cout << "[1] help\t" << endl;
    cout << "[2] myip\t" << endl;
    cout << "[3] myport\t" << endl;
    cout << "[4] connect <destination> <port no>\t" << endl;
    cout << "[5] list\t" << endl;
    cout << "[6] terminate <connection id>\t" << endl;
    cout << "[7] send <connection id> <message>\t" << endl;
    cout << "[0] exit\t" << endl;
}

void AppChat::displayHelp()
{
    system("clear");
    cout << "A Chat Application for Remote Message Exchange" << endl;
    cout << "[1] help  \tDisplay information about the available user interface options or command manual" << endl;
    cout << "[2] myip  \tDisplay the IP address of this process" << endl;
    cout << "[3] myport\tDisplay the port on which this process is listening for incoming connections" << endl;
    cout << "[4] connect <destination> <port no>\tThis command establishes a new TCP connection" << endl;
    cout << "                                   \t<destination>: The IP address of the computer" << endl;
    cout << "                                   \t<port no>: Port number" << endl;
    cout << "[5] list                           \tDisplay a numbered list of all the connections this process is part of" << endl;
    cout << "[6] terminate <connection id>      \tThis command will terminate the connection listed under the specified number" << endl;
    cout << "[7] send <connection id> <message> \tThis command will send the message to the host on the connection" << endl;
    cout << "                                   \t<connection id>: Connection id of machine" << endl;
    cout << "                                   \t<message>: The message to be sent" << endl;
    cout << "[0] exit                           \tExit program" << endl;
}

int AppChat::onStart()
{
    int option;
    int port;
    int connectionID;
    bool success = false;
    string destinationIP;

    while (true) {        
        displayMenu();
        cout << "Select your option: ";
        cin >> option;
        system("clear");

        switch (option)
        {
            case 1:
                displayHelp();
                break;

            case 2:
                connmgr->displayIPAdrress();
                break;

            case 3:
                connmgr->displayPortNumber();
                break;

            case 4:
                cout << "Enter the destination IP address: ";
                cin >> destinationIP;
                cout << "Enter the port number: ";
                cin >> port;

                connmgr->connectToDestination(destinationIP, port);

                break;

            case 5:
                connmgr->displayAllActiveConnection();
                break;

            case 6:
                cout << "Enter the connection ID: ";
                cin >> connectionID;

                success = connmgr->terminateConnection(connectionID);
                if (!success)
                    cout << "Connection id " << connectionID << " is not valid";
                break;

            case 7:
                cout << "Enter the connection ID: ";
                cin >> connectionID;
                
                connmgr->senDataToConnection(connectionID);

                break;

            case 0:
                /* code */
                cout << "Exit program";
                return 0;

            default:
                break;
        }
    }
}
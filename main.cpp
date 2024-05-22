#include "inc/AppChat.hpp"

int main(int argc, char* argv[])
{   
    if (argc != 2) {
		cout <<"Usage: " << argv[0] << " <port-number>" << endl;
		cout << "Please write the command correctly.\n" << endl;
        	return -1;
    }

    cout << "\nWelcome to chat application!" << endl;
    AppChat* app = AppChat::getInstance(atoi(argv[1]));
    app->onStart();

    return 0;
}
#ifndef __APPCHAT_H__
#define __APPCHAT_H__

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "ConnectionManager.hpp"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

class AppChat {
    private:
        static AppChat* _instance;
        static ConnectionManager* connmgr;
        AppChat();
    public:
        static AppChat* getInstance(int portno);
        void displayMenu();
        void displayHelp();
        int onStart();
};

#ifdef __cplusplus
}
#endif

#endif // __APPCHAT_H__
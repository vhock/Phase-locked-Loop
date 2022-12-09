#ifndef RPUTILITY_H
#define RPUTILITY_H

#ifdef _MSC_VER  //defines to replace posix queries
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif
#include <iostream>
#include <boost/asio.hpp>
#include<libssh/libssh.h>
#include<thread>

#include <stdlib.h>     //for using the function sleep

#include <QObject>

 class RPUtility : public QObject
{
    Q_OBJECT
public:
    RPUtility();
    static bool isValidIPAddress(std::string ipAddress);
    int connect(std::string ipAddress);
    void fireTestEvent();
    void monitorActiveSession();
    int disconnect();
 signals:
    void new_message(std::string message);
    void connectionStateChanged(int code);
private:
    ssh_session active_session;
    std::string last_message;
    int verify_knownhost(ssh_session rp_session);
    static int startConnection(ssh_session rp_session);
    int connection_status=0; //0 disconnected, 1 connected

};

#endif // RPUTILITY_H

#ifndef RPUTILITY_H
#define RPUTILITY_H

#ifdef _MSC_VER  //defines to replace posix queries
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif
#include <boost/asio.hpp>
#include<libssh/libssh.h>
#include<thread>
#include <iostream>
#include <conio.h>
#include <stdlib.h>     //for using the function sleep
#include <exception>
#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <QResource>
#include <Qfile>
// std::ifstream
 class RPUtility : public QObject
{
    Q_OBJECT
public:
    RPUtility();
    static bool isValidIPAddress(std::string ipAddress);
    int connect(std::string ipAddress);
    int disconnect();
    int sendCommand(std::string command,std::string &serverReply);
    int scp_helloworld();

 signals:
    void new_message(std::string message);
    void connectionStateChanged(int code);
private:
    ssh_session active_session;
    std::string last_message;
    int connection_status=0; //0 disconnected, 1 connected
    int verify_knownhost();
    int authenticate(ssh_session,std::string,std::string);
    int openChannel(ssh_session session);
    void monitorActiveSession();



};

#endif // RPUTILITY_H

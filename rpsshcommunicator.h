#ifndef RPSSHCOMMUNICATOR_H
#define RPSSHCOMMUNICATOR_H

#ifdef _MSC_VER  //defines to replace posix queries
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif
#include <boost/asio.hpp>
#include<libssh/libssh.h>
#include<thread>
#include <iostream>
//#include <conio.h>
#include <stdlib.h>     //for using the function sleep
#include <exception>
#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <QResource>
#include <QFile>
#include <map>
#include <array>
#include <bitset>
#include <cmath>
#include <QDebug>

class RPSSHCommunicator: public QObject
{
    Q_OBJECT
public:
    RPSSHCommunicator();
    int connect(std::string ipAddress,std::string user,std::string password);
    int disconnect();
    int sendCommand(std::string command,std::string &serverReply);

    void monitorActiveSession();
    void startMonitorActiveSession();

    int scp_copyBitfile();
    int executeBitfile();

    static bool isValidIPAddress(std::string ipAddress);


private:

    ssh_session active_session=NULL;
    std::string last_message;
    int connection_status=0; //0 disconnected, 1 connected

    static const std::string XDEVCFG_DIR;
    static const std::string PLL_BITFILE;
    static const std::string RP_MONITOR_COMMAND;
    static const std::string  TMPLOCATION;
    static const std::string RP_FILEXISTS_COMMAND;
    static const std::string  RP_EXECUTE_BITFILE_COMMAND;


    int verify_knownhost();
public slots:

signals:
    void ssh_log_message(std::string message);
    void ssh_connectionStateChanged(int code);

};

#endif // RPSSHCOMMUNICATOR_H

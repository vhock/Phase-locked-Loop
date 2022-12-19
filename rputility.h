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
#include <map>
#include <array>
#include <bitset>
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
    int scp_copyBitfile();
    int executeBitfile();
    int readParameter(std::string parameter,std::string &result,int pll );



private:
    static const std::string XDEVCFG_DIR;
    static const std::string PLL_BITFILE;
    static const std::string RP_MONITOR_COMMAND;
    static const std::string  TMPLOCATION;
    static const std::string RP_FILEXISTS_COMMAND;
    static const std::string  RP_EXECUTE_BITFILE_COMMAND;
    const std::map<std::string, std::array<int,4>> param_dict{{"2nd_harm", {0, 7, 7}},
                                                          {"pid_en",   {0, 6, 6}},
                                                          {"w_a",      {8, 15, 8}},
                                                          {"w_b",      {8, 7, 0}},
                                                          {"kp",       {0x10000, 31, 0}},
                                                          {"ki",       {0x10008, 31, 0}},
                                                          {"f0",       {0x20000, 31, 0}},
                                                          {"bw",       {0x20008, 31, 0}},
                                                          {"alpha",    {0x30000, 26, 10}},
                                                          {"order",    {0x30000, 2, 0}},
                                                          {"fNCO",     {0x40000,31,0}},//more a signal
                                                          {"fNCOErr",  {0x50000,31,0}}//more a signal
                                                         };



    const std::map<std::string, std::string>  output_options = {
                        { "PLL1", "000"               }   ,
                        { "PLL2", "001"               }   ,
                        { "PLL1 + PLL2", "010"        }   ,
                        { "PLL1 + IN2", "011"         }   ,
                        { "IN1", "100"                }   ,
                        { "IN2", "101"                }   ,
                        { "LI1_X", "110"              }   ,
                        { "LI2_Y", "111"              }   ,
                        };

    ssh_session active_session=NULL;
    std::string last_message;
    int connection_status=0; //0 disconnected, 1 connected
    int verify_knownhost();
    int authenticate(ssh_session,std::string,std::string);
    int openChannel(ssh_session session);
    void monitorActiveSession();
    int setParameter(std::string parameter,std::string value,int pll=0);
    void rescaleNegativeValues(float &val,int nbits);
public slots:
void pll1_f0_ChangedListener(int value);

signals:
    void log_message(std::string message);
    void connectionStateChanged(int code);


};

#endif // RPUTILITY_H

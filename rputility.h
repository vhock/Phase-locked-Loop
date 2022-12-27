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
#include <cmath>
#include <QDebug>


#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#include <rpparameterconverter.h>
// std::ifstream
class RPUtility : public QObject
{
    Q_OBJECT
public:
    RPUtility();
    static bool isValidIPAddress(std::string ipAddress);
    int connect(std::string ipAddress,std::string user,std::string password);
    int disconnect();
    int synchronizeParameters();
    int sendCommand(std::string command,std::string &serverReply);
    int scp_copyBitfile();
    int executeBitfile();
    template <typename T> T readParameterAsNumber(std::string parameter,int pll );
    void monitorActiveSession();

    int readParameter(std::string parameter,std::string &result,int pll );
   unsigned long readRegisterValueOfParameter(std::string parameter,int pll);
    void parameterChangedListener(std::string parameter,double value,int pll);
   bool logParameterChanges=false;//TODO should be private with getters/setters
  void startMonitorActiveSession();

private:
    static const std::string XDEVCFG_DIR;
    static const std::string PLL_BITFILE;
    static const std::string RP_MONITOR_COMMAND;
    static const std::string  TMPLOCATION;
    static const std::string RP_FILEXISTS_COMMAND;
    static const std::string  RP_EXECUTE_BITFILE_COMMAND;
    const std::map<std::string, std::array<int,3>> param_dict{{"2nd_harm", {0, 7, 7}},
                                                              {"pid_en",   {0, 6, 6}},
                                                              {"w_a",      {8, 15, 8}},
                                                              {"w_b",      {8, 7, 0}},
                                                              {"a",        {8, 0, 0}},// virtual parameter
                                                              {"phi",      {8, 0, 0}},//virtual parameter
                                                              {"kp",       {0x10000, 31, 0}},
                                                              {"ki",       {0x10008, 31, 0}},
                                                              {"f0",       {0x20000, 31, 0}},
                                                              {"bw",       {0x20008, 31, 0}},
                                                              {"alpha",    {0x30000, 26, 10}},
                                                              {"order",    {0x30000, 2, 0}},
                                                              {"fNCO",     {0x40000,31,0}},//more a signal
                                                              {"fNCOErr",  {0x50000,31,0}},//more a signal
                                                              {"output_1", {0, 2, 0}},
                                                              {"output_2", {0, 5, 3}}
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
    RPParameterConverter converter{};
    ulong shiftNegativeValueForWriting(long &val,int nbits);
    long shiftNegativeValueForReading( ulong &val,int nbits);
    ssh_session active_session=NULL;
    std::string last_message;
    int connection_status=0; //0 disconnected, 1 connected
    int verify_knownhost();
    int openChannel(ssh_session session);
    int setParameter(std::string parameter,std::string value,int pll=0);
    void logParameterChange(std::string parameter,int pll);
    // void rescaleNegativeValue(long &val,int nbits);
public slots:

signals:
    void log_message(std::string message);
    void connectionStateChanged(int code);
    void parameterInitialValue(std::string parameter,double value,int pll);// only once when the connection to the RP is made

};

#endif // RPUTILITY_H

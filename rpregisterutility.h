#ifndef RPPARAMETERCONVERTER_H
#define RPPARAMETERCONVERTER_H
#include <map>
#include <bitset>
#include <memory>
#include <QDebug>
#include <iostream>
#include <array>
/*
 * This class integrates, for parameters which share a register with other parameters, the parameter change into the the current register. The registers of this class should
 * depict the current state of the actual memory register on the Red Pitaya.
 *
 * */
class RPRegisterUtility
{
    friend class RPUtility;
public:
    RPRegisterUtility();
   void integrateParameter(int pll,std::string parameter,unsigned long value);
   unsigned long getParameterRegister(int pll,std::string parameter);
   bool verifyParameterRegisterMatch(int pll,std::string parameter,unsigned long receivedValue);//compare the received value for reading the register storing a certain parameter with the bitset mirroring this specific register
   unsigned long extractParameter(int pll,std::string parameter,const unsigned long &receivedValue);
private:
   //initialize the pll/parameter bitstream map
   std::bitset<32> pll1_reg_1; //ext_pins_n/p ,SEL B SEL A,2nd harmonic, PID_EN,
   std::bitset<32> pll1_reg_2;// W_A,W_B
   std::bitset<32> pll1_reg_3;//alpha, order

   std::bitset<32> pll2_reg_1; //ext_pins_n/p ,SEL B SEL A,2nd harmonic, PID_EN,
   std::bitset<32> pll2_reg_2;// W_A,W_B
   std::bitset<32> pll2_reg_3;//alpha, order
   ;
  std::map<int,std::map<std::string,std::bitset<32>*>> parameterTo32BitsetMap{   };
};
const std::map<std::string, std::array<int,3>> param_dict{{"2nd_harm", {0, 7, 7}},
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
                                                      {"fNCOErr",  {0x50000,31,0}},//more a signal
                                                      {"output_1", {0, 2, 0}},
                                                      {"output_2", {0, 5, 3}}
                                                     };

#endif // RPPARAMETERCONVERTER_H

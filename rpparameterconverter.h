#ifndef RPPARAMETERCONVERTER_H
#define RPPARAMETERCONVERTER_H
#include <map>
#include <bitset>
#include <memory>
#include <QDebug>
#include <iostream>
class RPParameterConverter
{
public:
    RPParameterConverter();
   void setParameter(int pll,std::string parameter,int value);
   long getParameter(int pll,std::string parameter);

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

#endif // RPPARAMETERCONVERTER_H

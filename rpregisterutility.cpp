// C++ GUI to interface with the RedPitaya based PLL
// Copyright (C) 2023 Vincent Hock
//  Based on work by Felix Tebbenjohanns, Domink Windey and Markus Rademacher

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.



#include "rpregisterutility.h"

/**
 * @brief RPParameterConverter::integrateParameter integrate a parameter into the representation of the 32-bit register it is stored in
 * @param pll
 * @param parameter
 * @param value
 */
void RPRegisterUtility::integrateParameter(int pll,std::string parameter,unsigned long value){
    try{
      int  msb = param_dict.at(parameter)[1];
      int  lsb = param_dict.at(parameter)[2];
      int nbits=param_dict.at(parameter)[1]-param_dict.at(parameter)[2]+1;
      std::bitset<32> paramRepresentaion(value); //not 32 bits actually but bitset cannot be allocated at runtime, so just ommit the rest

      std::bitset<32>* registerPtr= parameterTo32BitsetMap.at(pll).at(parameter);

      for (int i=0;i<nbits;i++){
          registerPtr->set(lsb+i,paramRepresentaion[i]);
      }

    }catch(std::out_of_range){
        int x=4;//TODO error handling
    }

}
/**
 * @brief RPParameterConverter::verifyParameterRegisterMatch
 *   compare the received value for reading the register storing a certain parameter with the bitset mirroring this specific register
 * @param pll
 * @param parameter
 * @param receivedValue
 * @return true if registers match, false otherwise
 */
bool RPRegisterUtility::verifyParameterRegisterMatch(int pll,std::string parameter,unsigned long receivedValue){
    unsigned long localRegisterValue=  getParameterRegister(pll,parameter);
    return localRegisterValue==receivedValue;
}
/**
 * @brief RPParameterConverter::integrateParameter extract a parameter from the 32-bit register it is stored in
 * @param pll
 * @param parameter
 * @param value
 */
unsigned long RPRegisterUtility::extractParameter(int pll,std::string parameter,const unsigned long &receivedValue){
    std::bitset<32> receivedValueAsBitset{receivedValue};
    std::bitset<32> extractedParameterBitset{};//parameter does not have 32 bit, but std::bitset needs a fixed size
    int  msb = param_dict.at(parameter)[1];
    int  lsb = param_dict.at(parameter)[2];
    int nbits=param_dict.at(parameter)[1]-param_dict.at(parameter)[2]+1;
    for (int i=0;i<nbits;i++){
        extractedParameterBitset.set(i,receivedValueAsBitset[lsb+i]);
    }
    return extractedParameterBitset.to_ulong();

}

RPRegisterUtility::RPRegisterUtility()
{

    //baseAddressTo32Bitset.emplace(0,{"w_a",std::make_shared<std::bitset<32>>(pll1_reg_2)});
    std::map<std::string,std::bitset<32>*> pll1map{
           {"output_1",(&pll1_reg_1)},
           {"output_2",(&pll1_reg_1)},
           {"ext_pins_p",(&pll1_reg_1)},
           {"ext_pins_n",(&pll1_reg_1)},
           {"2nd_harm",(&pll1_reg_1)},
           {"pid_en",(&pll1_reg_1)},

           {"w_a",(&pll1_reg_2)},
           {"w_b",(&pll1_reg_2)},

           {"alpha",(&pll1_reg_3)},
           {"order",(&pll1_reg_3)},

       };

       std::map<std::string,std::bitset<32>*> pll2map{
           {"output_1",(&pll2_reg_1)},
           {"output_2",(&pll2_reg_1)},
           {"ext_pins_p",(&pll2_reg_1)},
           {"ext_pins_n",(&pll2_reg_1)},
           {"2nd_harm",(&pll2_reg_1)},
           {"pid_en",(&pll2_reg_1)},

           {"w_a",(&pll2_reg_2)},
           {"w_b",(&pll2_reg_2)},

           {"alpha",(&pll2_reg_3)},
           {"order",(&pll2_reg_3)},

           };
    parameterTo32BitsetMap.emplace(0,pll1map);
    parameterTo32BitsetMap.emplace(1,pll2map);



}
/**
 * @brief RPParameterConverter::getParameterRegister Get the register which stores a certain parameter
 * @param pll
 * @param parameter
 * @return the register containing the parameter converted from a 32-bitset to an unsigned long value
 */
unsigned long RPRegisterUtility::getParameterRegister(int pll,std::string parameter){

   std::string bitsetval= parameterTo32BitsetMap.at(pll).at(parameter)->to_string();
 //   qDebug()<<"Bitset:"<<qPrintable(QString::fromStdString(bitsetval)); very useful but slow
     return  parameterTo32BitsetMap.at(pll).at(parameter)->to_ulong();
}

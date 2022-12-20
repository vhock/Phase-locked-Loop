#include "rpparameterconverter.h"


void RPParameterConverter::setParameter(int pll,std::string parameter,int value){
    try{
     std::bitset<32>* registerPtr= parameterTo32BitsetMap.at(pll).at(parameter);

     registerPtr->set();
    std::bitset<32> after= *registerPtr;

    qDebug()<<"test"<<qPrintable(QString::fromStdString(after.to_string()));
    }catch(std::out_of_range){
        int x=4;//TODO error handling
    }

}




RPParameterConverter::RPParameterConverter()
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

long RPParameterConverter::getParameter(int pll,std::string parameter){
    auto shr_ptrpiden=parameterTo32BitsetMap.at(pll).at("pid_en");
    auto shr_ptrp2nd=parameterTo32BitsetMap.at(pll).at("2nd_harm");

   std::string bitsetval= parameterTo32BitsetMap.at(pll).at(parameter)->to_string();
    qDebug()<<"Bitset:"<<qPrintable(QString::fromStdString(bitsetval));
     return  parameterTo32BitsetMap.at(pll).at(parameter)->to_ulong();
}

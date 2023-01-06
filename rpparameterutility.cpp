#include "rpparameterutility.h"

//constants
const std::string RPParameterUtility::XDEVCFG_DIR="/dev/xdevcfg";
const std::string RPParameterUtility::PLL_BITFILE="pll_project.bit";
const std::string RPParameterUtility::RP_MONITOR_COMMAND="/opt/redpitaya/bin/monitor ";
const std::string RPParameterUtility::TMPLOCATION="/tmp/";
const std::string RPParameterUtility::RP_FILEXISTS_COMMAND="test -f "+TMPLOCATION+PLL_BITFILE+"&& echo 1";
const std::string RPParameterUtility::RP_EXECUTE_BITFILE_COMMAND="cat "+TMPLOCATION+PLL_BITFILE+" >/dev/xdevcfg";

int pll_base_addr[2] = {0x41200000, 0x41300000};


/**
 * @brief RPUtility::shiftNegativeValueForWriting
 *  All Red Pitaya parameter values are postive, negative values are encoded by an offset relative to the maximum value
 * @param val
 * @param nbits
 * @return the always positive value as unsigned long
 */
ulong RPParameterUtility::shiftNegativeValueForWriting(long &val,int nbits){
    if (val<0){
        int64_t maxnbitvalue= static_cast<int64_t>(std::pow(2, nbits));
        ulong shifted=maxnbitvalue+val;
        return shifted;
    }
    else return val;
}
/**
 * @brief RPUtility::shiftNegativeValueForReading
 *  All Red Pitaya parameter values are postive, negative values are encoded by an offset relative to the maximum value
 *
 * @param val
 * @param nbits
 * @return the (potentially negative) value as signed long
 */
long RPParameterUtility::shiftNegativeValueForReading(ulong &val,int nbits){
    int64_t maxnbitvalue= static_cast<int64_t>(std::pow(2, nbits));
    int64_t maxnminus1bitvalue= static_cast<int64_t>(std::pow(2, nbits-1));

    if (val>maxnminus1bitvalue-1){
        long shifted=-(maxnbitvalue-val);
        return shifted;
    }
    else return val;
}

/**
 * @brief RPUtility::synchronizeParameters
 * Upon connection with the Red Pitaya, reads all parameters from the memory and sends them to UI for synchronization
 * @return
 */
int RPParameterUtility::synchronizeParameters(){
    validateRegisters=false; //registers do not match upon initial synchronization, no need to validate
    emit log_message("Updating parameters..");


    for (int pll=0;pll<2;pll++){
        for (auto const& elem : param_dict)
        {
            std::string parameter=elem.first;
            std::string value;
            if ((parameter=="output_1"||parameter=="output_2")&&pll==1){//bugfix
                continue;
            }
            readParameter(parameter,value,pll);
            int base_address=pll_base_addr[pll];
            int paramAddress=base_address+param_dict.at(parameter)[0];
            int nbits=param_dict.at(parameter)[1]-param_dict.at(parameter)[2]+1;
            float val_float = std::stof(value);
            long val_long{};
            ulong val_ulong{};
            std::string value_string{};


            if (parameter=="w_a"||parameter=="w_b"){
                val_long=std::stol(value);
                val_ulong=shiftNegativeValueForWriting(val_long,nbits);//val_long;//
            }

            if (parameter=="2nd_harm"||parameter=="pid_en"){
                val_ulong=std::stoul(value);
            }

            if (parameter=="output_1"||parameter=="output_2"){
                val_ulong=std::stoul(value);
            }



            if (parameter=="alpha"){
                float scaled_float = val_float* pow(2,17);
                val_ulong = static_cast<unsigned long>(scaled_float);
            }

            if (parameter=="order"){
                val_ulong=static_cast<unsigned long>(val_float)-1;

            }



            //integration into registers for certain parameters
            if (parameter=="alpha"||parameter=="order"||parameter=="output_1"
                    ||parameter=="output_2"||parameter=="2nd_harm"
                    ||parameter=="pid_en"||parameter=="w_a"||parameter=="w_b"){
                converter.integrateParameter(pll,parameter,val_ulong); //integrate the parameter into register because it is shared with other parameters
                unsigned long integratedValue=converter.getParameterRegister(pll,parameter);//get the full register as a long representation
                val_ulong=integratedValue;
            }

           // qDebug()<<"Parameter "<<qPrintable(QString::fromStdString(parameter+" has the value "+value));
            emit parameterInitialValue(parameter,std::stod(value),pll);


        }

    }
    emit log_message("All parameters updated.");
    validateRegisters=true; //restore this boolean so following parameter changes will validate the register
    return 0;
}
/**
 * @brief RPParameterUtility::setParameter
 * Set parameter to a certain value for a certain PLL
 * @param parameter
 * @param value
 * @param pll
 * @return
 */
int RPParameterUtility::setParameter(std::string parameter,std::string value,int pll ){
    try{
        int base_address=pll_base_addr[pll];
        int paramAddress=base_address+param_dict.at(parameter)[0];
        int nbits=param_dict.at(parameter)[1]-param_dict.at(parameter)[2]+1;
        float val_float = std::stof(value);



        long val_long{};
        ulong val_ulong{};
        std::string value_string{};
        //encode the parameters a and phi into weights w_a and w_b. Set these values.
        if (parameter=="a"||parameter=="phi"){ //TODO this has huge rounding errors, is there a better way to do it?
            std::string w_a{};
            std::string w_b{};
            readParameter("w_a",w_a,pll);
            readParameter("w_b",w_b,pll);
            double a= sqrt(pow(std::stol(w_a),2)+pow(std::stol(w_b),2)); //current amplitude
            double phi=atan2(std::stol(w_a),std::stol(w_b));//current phase
            if (parameter=="a"){//a gets a new value
                a=std::stoul(value);
            }else if (parameter=="phi"){ //phi gets a new value
                phi=std::stof(value)/360*(2*M_PI);
            }
            long w_a_long=a*sin(phi);
            long w_b_long=a*cos(phi);
            setParameter("w_a",std::to_string(w_a_long),pll);
            setParameter("w_b",std::to_string(w_b_long),pll);
            return 0;


        }

        if (parameter=="w_a"||parameter=="w_b"){
            val_long=std::stol(value);
            val_ulong=shiftNegativeValueForWriting(val_long,nbits);//val_long;//
        }

        if (parameter=="2nd_harm"||parameter=="pid_en"){
            val_ulong=std::stoul(value);
        }

        if (parameter=="output_1"||parameter=="output_2"){
            val_ulong=std::stoul(value);
        }
        if (parameter=="ext_pins_n"||parameter=="ext_pins_p"){
            //TODO these do not work in the original either it seems. omitted for now
        }

        if (parameter=="f0"||parameter=="bw"){
            float scaled_float = val_float/(31.25*pow(10,6))* pow(2,32);
            val_ulong = static_cast<unsigned long>(scaled_float);
            int secondHarmOn=readParameterAsNumber<int>("2nd_harm",pll);
            val_ulong=val_ulong*(1+secondHarmOn);

        }
        if (parameter=="kp"||parameter=="ki"){
            float scaled_float = val_float* pow(2,16);
            val_long = static_cast< long>(scaled_float);
            val_ulong= shiftNegativeValueForWriting(val_long,nbits);
        }

        if (parameter=="alpha"){
            float scaled_float = val_float* pow(2,17);
            val_ulong = static_cast<unsigned long>(scaled_float);
        }

        if (parameter=="order"){
            val_ulong=static_cast<unsigned long>(val_float)-1;

        }



        //integration into registers for certain parameters
        if (parameter=="alpha"||parameter=="order"||parameter=="output_1"
                ||parameter=="output_2"||parameter=="2nd_harm"
                ||parameter=="pid_en"||parameter=="w_a"||parameter=="w_b"){
            converter.integrateParameter(pll,parameter,val_ulong); //integrate the parameter into register because it is shared with other parameters
            unsigned long integratedValue=converter.getParameterRegister(pll,parameter);//get the full register as a long representation
            val_ulong=integratedValue;
        }




        value_string=std::to_string(val_ulong);
        std::string valueSetCommand=RP_MONITOR_COMMAND+std::to_string(paramAddress)+" ";
        valueSetCommand.append(value_string );
        std::string reply{};

        sshCommunicator->sendCommand(valueSetCommand,reply);
        return 0;// no issues
    }catch(std::exception &ex){
        emit  log_message("Setting parameter "+parameter+" failed:"+ex.what());
        return -1;
    }

}
template <typename T>
T RPParameterUtility::readParameterAsNumber(std::string parameter,int pll ){
    std::string parameterValueAsString{};
    readParameter(parameter,parameterValueAsString,pll);
    double valAsDouble=  std::stod(parameterValueAsString);//this should hopefully be safe
    try{
        return (T)valAsDouble; //cast to whatever value is wanted
    }catch(...){
        emit log_message("Casting number failed ");
        return -1;
    }
}




/**
 * @brief RPParameterUtility::readParameter
 * Read a parameter from the the Red Pitaya.Depending on the type of parameter, this method will return a int or float value as a string
 * @param parameter
 * @param result
 * @param pll
 * @return
 */
int RPParameterUtility::readParameter(std::string parameter,std::string &result,int pll ){
    try{
        int nbits=param_dict.at(parameter)[1]-param_dict.at(parameter)[2]+1;

        unsigned long registerValue=readRegisterValueOfParameter(parameter,pll);//read register value at the address of the parameter
        long parameterValue{};

        if (parameter=="a"){
            std::string w_a{};
            std::string w_b{};
            readParameter("w_a",w_a,pll);
            readParameter("w_b",w_b,pll);
            long w_a_signed=std::stol(w_a);
            long w_b_signed=std::stol(w_b);

            unsigned long a= sqrt(pow(w_a_signed,2)+pow(w_b_signed,2));
            result=std::to_string(a);
            return 0;
        }
        if (parameter=="phi"){
            std::string w_a{};
            std::string w_b{};
            readParameter("w_a",w_a,pll);
            readParameter("w_b",w_b,pll);
            long w_a_signed=std::stol(w_a);
            long w_b_signed=std::stol(w_b);
            long phi= atan2(w_a_signed,w_b_signed)/(2*M_PI)*360;
            result=std::to_string(phi);
            return 0;
        }


        if (parameter=="f0"||parameter=="bw"){
            long scaledReply=registerValue/pow(2,32) *31.25*pow(10,6);
            int secondHarmOn=readParameterAsNumber<int>("2nd_harm",pll);
            long scaledFor2ndHarmonic=scaledReply/(1+secondHarmOn);
            result=std::to_string(scaledFor2ndHarmonic);
            return 0;

        }
        if (parameter=="kp"||parameter=="ki"){
            long shiftedValue= shiftNegativeValueForReading(registerValue,nbits);
            double scaledReply=shiftedValue/pow(2,16);
            result=std::to_string(scaledReply);
            return 0;

        }


        //these have to be extracted from a register because they do not span the entire 32 bits
        if (parameter=="alpha"||parameter=="order"||parameter=="output_1"
                ||parameter=="output_2"||parameter=="2nd_harm"
                ||parameter=="pid_en"||parameter=="w_a"||parameter=="w_b"){
            //perform check first
            if (validateRegisters){
                bool registerInSync= converter.verifyParameterRegisterMatch(pll,parameter,registerValue);//the RPParameterConverter class mirrors the Red Pitaya registers. Check that the corresponding register of the parameter is identical to the one received from the Red Pitaya
                if (!registerInSync){
                    emit log_message("Client-host register mismatch for parameter "+ parameter);
                    // return -1;
                } //TODO reactivate
            }
            unsigned long extractedParameterValue=converter.extractParameter(pll,parameter,registerValue);
            if (parameter=="w_a"||parameter=="w_b"){
                parameterValue=shiftNegativeValueForReading(extractedParameterValue,nbits);
            }else {
                parameterValue=extractedParameterValue;
            }
            result=std::to_string(parameterValue);

        }

        if (parameter=="alpha"){
            double scaled_double = parameterValue/ pow(2,17);
            result=std::to_string(scaled_double);
            return 0;


        }

        if (parameter=="order"){
            int val_int=static_cast<unsigned long>(parameterValue)+1;
            result=std::to_string(val_int);
            return 0;



        }

        if (result.empty()){
            result=std::to_string(-MAXINT);
            emit  log_message("Reading parameter "+parameter+" failed.");
            return -1;
        }

        return 0;
    }
    catch(std::exception &ex){
        emit  log_message("Reading parameter "+parameter+" failed:"+ex.what());
        return -1;
    }
}



/**
 * @brief RPParameterUtility::logParameterChange
 * Send a log message about the parameter change
 * @param parameter
 * @param pll
 */
void RPParameterUtility::logParameterChange(std::string parameter,int pll){
    std::string result{};

    readParameter(parameter,result,pll);
    emit log_message("Changed parameter "+parameter+" to:"+ result);
}





/**
 * @brief RPParameterUtility::parameterChangedListener
 * Handles parameter change events sent from the UI
 * @param parameter
 * @param value
 * @param pll
 */
void RPParameterUtility::parameterChangedListener(std::string parameter,double value,int pll){
    setParameter(parameter,std::to_string(value),pll);
    //verify the parameter has been set by reading the parameter and emitting the parameter as log message
    if (logParameterChanges){
        std::thread logChange(&RPParameterUtility::logParameterChange,this,parameter,pll);
        logChange.join();
    }
    //logChange.detach();

}


/**
 * @brief RPUtility::readRegisterValue reads the value of the 32-bit register in which the parameter is stored and returns it in unsigned long format
 * @param parameter
 * @param pll
 * @return
 */
unsigned long RPParameterUtility::readRegisterValueOfParameter(std::string parameter,int pll){
    int base_address=pll_base_addr[pll];
    int paramAddress=base_address+param_dict.at(parameter)[0];
    std::string registerReadCommand=RP_MONITOR_COMMAND+std::to_string(paramAddress);
    int nbits=param_dict.at(parameter)[1]-param_dict.at(parameter)[2]+1;
    std::string reply{};
    int success=sshCommunicator->sendCommand(registerReadCommand,reply); //read register value at the address of the parameter

    if (reply.empty()||success!=0){
        emit log_message("Failed to read parameter "+parameter);
        return 999;
    }

    unsigned long registerValue=std::stoul( reply,0,16 );
    return registerValue;
}

/**
 * @brief parameterInitialValue emits the value of a parameter upon connection for the UI to deal with it
 * @param parameter
 * @param value
 * @param pll
 */
void parameterInitialValue(std::string parameter,double value,int pll){

}




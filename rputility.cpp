#include "rputility.h"

//constants
const std::string RPUtility::XDEVCFG_DIR="/dev/xdevcfg";
const std::string RPUtility::PLL_BITFILE="pll_project.bit";
const std::string RPUtility::RP_MONITOR_COMMAND="/opt/redpitaya/bin/monitor ";
const std::string RPUtility::TMPLOCATION="/tmp/";
const std::string RPUtility::RP_FILEXISTS_COMMAND="test -f "+TMPLOCATION+PLL_BITFILE+"&& echo 1";
const std::string RPUtility::RP_EXECUTE_BITFILE_COMMAND="cat "+TMPLOCATION+PLL_BITFILE+" >/dev/xdevcfg";

int pll_base_addr[2] = {0x41200000, 0x41300000};

RPUtility::RPUtility()
{
}

/* All Red Pitaya parameter values are postive, negative values are encoded by an offset relative to the maximum value
 */
ulong RPUtility::shiftNegativeValueForWriting(long &val,int nbits){
    if (val<0){
        int64_t maxnbitvalue= static_cast<int64_t>(std::pow(2, nbits));
        ulong shifted=maxnbitvalue+val;
        return shifted;
    }
    else return val;
}

long RPUtility::shiftNegativeValueForReading(ulong &val,int nbits){
    int64_t maxnbitvalue= static_cast<int64_t>(std::pow(2, nbits));
    int64_t maxnminus1bitvalue= static_cast<int64_t>(std::pow(2, nbits-1));

    if (val>maxnminus1bitvalue-1){
        long shifted=-(maxnbitvalue-val);
        return shifted;
    }
    else return val;
}

int RPUtility::synchronizeParameters(){


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

            qDebug()<<"Parameter "<<qPrintable(QString::fromStdString(parameter+" has the value "+value));
            emit parameterInitialValue(parameter,std::stod(value),pll);


        }

    }
    emit log_message("All parameters updated.");
    return 0;
}

int RPUtility::setParameter(std::string parameter,std::string value,int pll ){
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

        sendCommand(valueSetCommand,reply);
        return 0;// no issues
    }catch(std::exception &ex){
        emit  log_message("Setting parameter "+parameter+" failed:"+ex.what());
    }

}
template <typename T>
T RPUtility::readParameterAsNumber(std::string parameter,int pll ){
    std::string parameterValueAsString{};
    readParameter(parameter,parameterValueAsString,pll);
    double valAsDouble=  std::stod(parameterValueAsString);//this should hopefully be safe
    try{
        return (T)valAsDouble; //cast to whatever value is wanted
    }catch(_exception &ex){
        log_message("Casting number failed");
    }
}




/*Depending on the type of parameter, this method will return a int or float value as a string
 * */
int RPUtility::readParameter(std::string parameter,std::string &result,int pll ){
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
            //            bool registerInSync= converter.verifyParameterRegisterMatch(pll,parameter,registerValue);//the RPParameterConverter class mirrors the Red Pitaya registers. Check that the corresponding register of the parameter is identical to the one received from the Red Pitaya
            //            if (!registerInSync){
            //                emit log_message("Client-host register mismatch for address"+ std::to_string(paramAddress));
            //                return -1;
            //            } //TODO reactivate

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


int RPUtility::sendCommand(std::string command,std::string &serverReply){
    int rc;
    char buffer[1];
    int nbytes;
    if (active_session==NULL||connection_status!=1){
        emit log_message("No active connection, sending command "+command+" failed.");
        return -1;
    }

    ssh_channel  channel = ssh_channel_new(active_session);
    ssh_channel_set_blocking(channel,1); //important for the ssh channel actually to actually wait for the reply
    std::string receive = "";

    int sessionOK=ssh_channel_open_session(channel);
    if (sessionOK == SSH_ERROR){
        emit log_message(ssh_get_error(active_session));
        return -1;
    }

    rc = channel_request_exec(channel, command.c_str());
    if (rc ==SSH_ERROR) {
        emit log_message(ssh_get_error(active_session));
        return -1;
    }
    //channel_read(channel, buffer, sizeof(buffer),0);
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0); //TODO how about is_stderr=1?
    while (nbytes > 0)
    {
        receive.append(buffer, nbytes);
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (!receive.empty() && receive[receive.length()-1] == '\n') { //remove the newline
        receive.erase(receive.length()-1);
    }
    // emit log_message("Buffer content:");
    // emit log_message(receive);

    //Cleanup
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    serverReply=receive;
    return SSH_OK;
}

void RPUtility::logParameterChange(std::string parameter,int pll){
    std::string result{};

    readParameter(parameter,result,pll);
    emit log_message("Changed parameter "+parameter+" to:"+ result);
}

bool RPUtility::isValidIPAddress(std::string ipAddress){
    boost::system::error_code ec;
    boost::asio::ip::address::from_string( ipAddress, ec );
    if ( ec ){
        return false;
    }
    return true;
}

int RPUtility::verify_knownhost()
{
    enum ssh_known_hosts_e state;
    unsigned char* hash = NULL;
    ssh_key srv_pubkey = NULL;
    size_t hlen;
    char buf[10];
    char* hexa;
    char* p;
    int cmp;
    int rc;
    rc = ssh_get_server_publickey(active_session, &srv_pubkey);
    if (rc < 0) {
        return -1;
    }
    rc = ssh_get_publickey_hash(srv_pubkey,
                                SSH_PUBLICKEY_HASH_SHA1,
                                &hash,
                                &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return -1;
    }
    state = ssh_session_is_known_server(active_session);
    switch (state) {
    case SSH_KNOWN_HOSTS_OK:
        /* OK */
        break;
    case SSH_KNOWN_HOSTS_CHANGED:
        fprintf(stderr, "Host key for server changed: it is now:\n");
        ssh_print_hexa("Public key hash", hash, hlen);
        fprintf(stderr, "For security reasons, connection will be stopped\n");
        ssh_clean_pubkey_hash(&hash);
        return -1;
    case SSH_KNOWN_HOSTS_OTHER:
        fprintf(stderr, "The host key for this server was not found but an other"
                        "type of key exists.\n");
        fprintf(stderr, "An attacker might change the default server key to"
                        "confuse your client into thinking the key does not exist\n");
        ssh_clean_pubkey_hash(&hash);
        return -1;
    case SSH_KNOWN_HOSTS_NOT_FOUND:
        fprintf(stderr, "Could not find known host file.\n");
        fprintf(stderr, "If you accept the host key here, the file will be"
                        "automatically created.\n");
        /* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_KNOWN_HOSTS_UNKNOWN:
        hexa = ssh_get_hexa(hash, hlen);
        fprintf(stderr, "The server is unknown. Do you trust the host key?y/n\n");
        fprintf(stderr, "Public key hash: %s\n", hexa);
        ssh_string_free_char(hexa);
        ssh_clean_pubkey_hash(&hash);
        p = fgets(buf, sizeof(buf), stdin);
        if (p == NULL) {
            return -1;
        }
        cmp = strncasecmp(buf, "y", 1);
        if (cmp != 0) {
            return -1;
        }
        rc = ssh_session_update_known_hosts(active_session);
        if (rc < 0) {
            fprintf(stderr, "Error %s\n", strerror_s(buf,800,errno));
            return -1;
        }
        break;
    case SSH_KNOWN_HOSTS_ERROR:
        fprintf(stderr, "Error %s", ssh_get_error(active_session));
        ssh_clean_pubkey_hash(&hash);
        return -1;
    }
    ssh_clean_pubkey_hash(&hash);
    return 0;
}


int RPUtility::connect(std::string ipAddress,std::string user,std::string password){
    emit log_message("Establishing connection to "+ipAddress);
    ssh_session rp_session = ssh_new();
    if (rp_session == NULL) {
        exit(-1);
    }
    ssh_options_set(rp_session, SSH_OPTIONS_HOST, ipAddress.c_str()); //red pitaya address, needs to be replaced by prompt later
    ssh_options_set(rp_session, SSH_OPTIONS_LOG_VERBOSITY, "3");//

    int returnValue=   ssh_connect(rp_session);
    if (returnValue == SSH_OK) {
        emit log_message("Successfully connected to "+ipAddress+"...");
    }
    else {
        emit log_message("Connection to "+ipAddress+" failed."+std::string(ssh_get_error(rp_session)));
        return -1;//no need to continue code execution, TODO error handling
    }

    //TODO skipped for now, but has to be done..
    // verify_knownhost(rp_session);

    //int hostVerifcation = verify_knownhost(rp_session);//check if the host is known for security
    //  if (hostVerifcation != 0) {
    //      return -1; //verification failed
    //  }

    //Authentication
    int auth=ssh_userauth_password(rp_session,user.c_str(),password.c_str());
    if (auth != SSH_AUTH_SUCCESS)
    {
        emit log_message("Authentication failed:"+std::string(ssh_get_error(rp_session)));
        ssh_disconnect(rp_session);
        ssh_free(rp_session);
        return -1;
    }else {
        emit log_message("Authentication successful:"+std::string(ssh_get_error(rp_session)));

    }

    active_session=rp_session; //copy construction, important because if ref to rp_session is used all other threads works with undefined memory

    emit connectionStateChanged(1);
    connection_status=1;

    return 0;
}


void RPUtility::startMonitorActiveSession(){
    std::thread monitorSessionThread(&RPUtility::monitorActiveSession,this);// launch a thread which monitors the active session
    monitorSessionThread.detach();
}


//TODO buggy, disabled for now
int RPUtility::disconnect(){
    if (active_session!=nullptr&&connection_status==1){
        try{
            int sessionstatus=ssh_get_status(active_session);
            ssh_disconnect(active_session);
            ssh_free(active_session);
            connection_status=0;
            emit connectionStateChanged(connection_status);
            emit log_message("Disconnected");
            return 0;
        }catch (...){
            int x=4;
        }
    }
    return -1;
}


//never launch me in an undetached thread
/*
 * This method checks if the connection to the Red Pitaya is still active, every 5 seconds
 * In debug mode, this method may cause issues
 */
void RPUtility::monitorActiveSession(){
    int isConnected=1;//when this method is called, connection_status is equal 1
    while (true){
        ssh_channel channel;
        int rc;
        channel = ssh_channel_new(active_session);
        if (ssh_channel_is_closed(channel)==0){
            return ;
        }
        rc = ssh_channel_open_session(channel);
        if (rc == SSH_ERROR){//connection lost
            isConnected=0;
            emit log_message(std::string(ssh_get_error(active_session)));

        }else {//all good
            isConnected=1;
            ssh_channel_close(channel);
            ssh_channel_free(channel);
        }
        //something changed, emit the according message
        if (isConnected!=connection_status){
            connection_status=isConnected;
            emit connectionStateChanged(connection_status);
            if (connection_status==0){
                emit log_message("Connection lost:"+std::to_string(rc));
            }else if (connection_status==1){
                emit log_message("Connection established:"+std::to_string(rc));
            }
        }
        Sleep(5000); //checking on the connection every few seconds is enough

    }
}

int RPUtility::executeBitfile(){
    //verify that the file exists
    try{


        std::string testCommand=RP_FILEXISTS_COMMAND;
        std::string reply("");
        sendCommand(RP_FILEXISTS_COMMAND,reply);
        if (reply.empty()){
            return -1; //file not found
        }
        else {
            sendCommand(RP_EXECUTE_BITFILE_COMMAND,reply);
        }
        emit log_message("Executed bitfile "+PLL_BITFILE);}
    catch(...){
        emit log_message("Executing Bitfile failed");
    }
}

int RPUtility::scp_copyBitfile()
{   try{
        if( this->connection_status!=1){//no active connection
            emit log_message("No active connection");
            return -1;
           }
        ssh_scp scp;
        int rc;

        scp = ssh_scp_new
                (active_session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, "/tmp/");
        if (scp == NULL)
        {
            fprintf(stderr, "Error allocating scp session: %s\n",
                    ssh_get_error(active_session));
            return SSH_ERROR;
        }
        int status=ssh_get_status(active_session) ;
        rc = ssh_scp_init(scp);
        if (rc != SSH_OK)
        {
            emit log_message("Error initializing scp session: %s\n"+
                             std::string(ssh_get_error(active_session)));
            fprintf(stderr, "Error initializing scp session: %s\n",
                    ssh_get_error(active_session));
            ssh_scp_free(scp);
            return rc;
        }

        //read local bit file into bitstring
        std::string filePath=":/"+PLL_BITFILE;
        QString qFilePath =QString::fromStdString(filePath);
        QFile knightRider(qFilePath);
        bool kRxists=knightRider.exists();
        knightRider.open(QIODevice::ReadOnly); //TODO safety
        QByteArray blob = knightRider.readAll();


        int length=blob.length();

        //knight rider
        rc = ssh_scp_push_file
                (scp, PLL_BITFILE.c_str(), length, 0400 |  0200);
        if (rc != SSH_OK)
        {
            emit log_message("Can't open remote file: %s\n"+
                             std::string(ssh_get_error(active_session)));
            return rc;
        }

        rc = ssh_scp_write(scp, blob, length);
        if (rc != SSH_OK)
        {
            emit log_message( "Can't write to remote file: %s\n"+
                              std::string(ssh_get_error(active_session)));
            return rc;
        }
        //nothing went wrong
        emit log_message("Succesfully copied bitfile "+PLL_BITFILE+" to "+"/tmp/");
        return SSH_OK;}
    catch (...){
        emit log_message("Could not copy bitfile");
    }
}




void RPUtility::parameterChangedListener(std::string parameter,double value,int pll){
    setParameter(parameter,std::to_string(value),pll);
    //verify the parameter has been set by reading the parameter and emitting the parameter as log message
    if (logParameterChanges){
        std::thread logChange(&RPUtility::logParameterChange,this,parameter,pll);
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
unsigned long RPUtility::readRegisterValueOfParameter(std::string parameter,int pll){
    int base_address=pll_base_addr[pll];
    int paramAddress=base_address+param_dict.at(parameter)[0];
    std::string registerReadCommand=RP_MONITOR_COMMAND+std::to_string(paramAddress);
    int nbits=param_dict.at(parameter)[1]-param_dict.at(parameter)[2]+1;
    std::string reply{};
    sendCommand(registerReadCommand,reply); //read register value at the address of the parameter
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




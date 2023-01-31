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





#include "rpsshcommunicator.h"

const std::string RPSSHCommunicator::XDEVCFG_DIR="/dev/xdevcfg";
const std::string RPSSHCommunicator::PLL_BITFILE="pll_project.bit";
const std::string RPSSHCommunicator::RP_MONITOR_COMMAND="/opt/redpitaya/bin/monitor ";
const std::string RPSSHCommunicator::TMPLOCATION="/tmp/";
const std::string RPSSHCommunicator::RP_FILEXISTS_COMMAND="test -f "+TMPLOCATION+PLL_BITFILE+"&& echo 1";
const std::string RPSSHCommunicator::RP_EXECUTE_BITFILE_COMMAND="cat "+TMPLOCATION+PLL_BITFILE+" >/dev/xdevcfg";



RPSSHCommunicator::RPSSHCommunicator()
{     

}


int RPSSHCommunicator::sendCommand(std::string command,std::string &serverReply){
    int rc;
    char buffer[1];
    int nbytes;
    if (active_session==NULL||connection_status!=1){
        emit ssh_log_message("No active connection, sending command "+command+" failed.");
        return -1;
    }

    ssh_channel  channel = ssh_channel_new(active_session);
    ssh_channel_set_blocking(channel,1); //important for the ssh channel actually to actually wait for the reply
    std::string receive = "";

    int sessionOK=ssh_channel_open_session(channel);
    if (sessionOK == SSH_ERROR){
        emit ssh_log_message(ssh_get_error(active_session));
        return -1;
    }

    rc = channel_request_exec(channel, command.c_str());
    if (rc ==SSH_ERROR) {
        emit ssh_log_message(ssh_get_error(active_session));
        return -1;
    }
    //channel_read(channel, buffer, sizeof(buffer),0);
    if(ssh_channel_get_exit_status(channel)!=0){
        emit ssh_log_message("SSH channel error status when sending command "+command);
        return -1;
    }

    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0); //TODO how about is_stderr=1?
    while (nbytes > 0)
    {
        receive.append(buffer, nbytes);
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (!receive.empty() && receive[receive.length()-1] == '\n') { //remove the newline
        receive.erase(receive.length()-1);
    }
    // emit ssh_log_message("Buffer content:");
    // emit ssh_log_message(receive);

    //Cleanup
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    serverReply=receive;
    return SSH_OK;
}


/**
 * @brief RPSSHCommunicator::isValidIPAddress
 * validates if a certain string is a valid IP address
 * @param ipAddress
 * @return true if valid, false otherwise
 */
bool RPSSHCommunicator::isValidIPAddress(std::string ipAddress){
    boost::system::error_code ec;
    boost::asio::ip::address::from_string( ipAddress, ec );
    if ( ec ){
        return false;
    }
    return true;
}

int RPSSHCommunicator::verify_knownhost()
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

/**
 * @brief RPSSHCommunicator::connect
 * Connect to the given IP address via SSH
 * @param ipAddress
 * @param user
 * @param password
 * @return 0 if successful, -1 otherwise
 */
int RPSSHCommunicator::connect(std::string ipAddress,std::string user,std::string password){
    emit ssh_log_message("Establishing connection to "+ipAddress);
    ssh_session rp_session = ssh_new();
    if (rp_session == NULL) {
        exit(-1);
    }
    ssh_options_set(rp_session, SSH_OPTIONS_HOST, ipAddress.c_str()); //red pitaya address, needs to be replaced by prompt later
    ssh_options_set(rp_session, SSH_OPTIONS_LOG_VERBOSITY, "3");//

    int returnValue=   ssh_connect(rp_session);
    if (returnValue == SSH_OK) {
        emit ssh_log_message("Successfully connected to "+ipAddress+"...");
    }
    else {
        emit ssh_log_message("Connection to "+ipAddress+" failed."+std::string(ssh_get_error(rp_session)));
        return -1;//no need to continue code execution, TODO error handling
    }

    //TODO skipped for now, but has to be done..
    // verify_knownhost(rp_session);

    //int hostVerifcation = verify_knownhost(rp_session);//check if the host is known for security
    //  if (hostVerifcation != 0) {
    //      return -1; //verification failed
    //  }

    //Authentication
    emit ssh_log_message("Authenticating..");

    int auth=ssh_userauth_password(rp_session,user.c_str(),password.c_str());
    if (auth != SSH_AUTH_SUCCESS)
    {
        emit ssh_log_message("Authentication failed:"+std::string(ssh_get_error(rp_session)));
        ssh_disconnect(rp_session);
        ssh_free(rp_session);
        return -1;
    }else {
        emit ssh_log_message("Authentication successful."+std::string(ssh_get_error(rp_session)));

    }

    active_session=rp_session; //copy construction, important because if ref to rp_session is used all other threads works with undefined memory

    emit ssh_connectionStateChanged(1);
    connection_status=1;

    return 0;
}

/**
 * @brief RPSSHCommunicator::startMonitorActiveSession
 * Just for starting a detached thread
 */
void RPSSHCommunicator::startMonitorActiveSession(){
    std::thread monitorSessionThread(&RPSSHCommunicator::monitorActiveSession,this);// launch a thread which monitors the active session
    monitorSessionThread.detach();
}


/**
 * @brief RPSSHCommunicator::disconnect
 * Disconnect from the active session
 * @return 0 if disconnection successful, -1 otherwise
 */
int RPSSHCommunicator::disconnect(){
    if (active_session!=nullptr&&connection_status==1){
        try{
            int sessionstatus=ssh_get_status(active_session);
            ssh_disconnect(active_session);
            ssh_free(active_session);
            connection_status=0;
            emit ssh_connectionStateChanged(connection_status);
            emit ssh_log_message("Disconnected");
            return 0;
        }catch (...){
            emit ssh_log_message("Error while disconnecting");

            int x=4;
        }
    }
    return -1;
}



/**
 * @brief RPSSHCommunicator::monitorActiveSession
 * This method checks if the connection to the Red Pitaya is still active, every 5 seconds
 * In debug mode, this method may cause issues
 * Never launch in undetached thread!
 */
void RPSSHCommunicator::monitorActiveSession(){
    int isConnected=1;//when this method is called, connection_status is equal 1
    while (connection_status==1){
        ssh_channel channel;
        int rc;
        channel = ssh_channel_new(active_session);
        if (ssh_channel_is_closed(channel)==0){
            return ;
        }
        rc = ssh_channel_open_session(channel);
        if (rc == SSH_ERROR){//connection lost
            isConnected=0;
            emit ssh_log_message(std::string(ssh_get_error(active_session)));


        }else {//all good
            isConnected=1;
            ssh_channel_close(channel);
            ssh_channel_free(channel);
        }
        //something changed, emit the according message
        if (isConnected!=1){
            connection_status=isConnected;
            emit ssh_connectionStateChanged(connection_status);
            if (connection_status==0){
                emit ssh_log_message("Connection lost:"+std::to_string(rc));
                return;
            }
        }
        Sleep(5000); //checking on the connection every few seconds is enough

    }
}

/**
 * @brief RPSSHCommunicator::executeBitfile
 * Launch the bitfile
 * @return
 */
int RPSSHCommunicator::executeBitfile(){
    //verify that the file exists
    try{
        std::string testCommand=RP_FILEXISTS_COMMAND;
        std::string reply("");
        sendCommand(RP_FILEXISTS_COMMAND,reply);
        if (reply.empty()){
            emit ssh_log_message(PLL_BITFILE+" not found");
            return -1; //file not found
        }
        else {
            int executed= sendCommand(RP_EXECUTE_BITFILE_COMMAND,reply);
            if (executed==0){
                emit ssh_log_message("Executed bitfile "+PLL_BITFILE);
                return 0;
            }

        }
    }
    catch(...){
        emit ssh_log_message("Executing Bitfile failed");
        return -1;
    }
}

/**
 * @brief RPSSHCommunicator::scp_copyBitfile
 * copy the bitfile to the red pitaya via SSH
 * @return
 */
int RPSSHCommunicator::scp_copyBitfile()
{   try{
        if( this->connection_status!=1){//no active connection
            emit ssh_log_message("No active connection");
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
            emit ssh_log_message("Error initializing scp session: %s\n"+
                                 std::string(ssh_get_error(active_session)));
            fprintf(stderr, "Error initializing scp session: %s\n",
                    ssh_get_error(active_session));
            ssh_scp_free(scp);
            return rc;
        }

        //read local bit file into bitstring
        std::string filePath=":/"+PLL_BITFILE;
        QString qFilePath =QString::fromStdString(filePath);
        QFile bitFile(qFilePath);
        bool kRxists=bitFile.exists();
        if (!kRxists){
            emit ssh_log_message("Error parsing bitfile");
            return -1;
        }
        bitFile.open(QIODevice::ReadOnly); //TODO safety
        QByteArray blob = bitFile.readAll();


        int length=blob.length();

        //knight rider
        rc = ssh_scp_push_file
                (scp, PLL_BITFILE.c_str(), length, 0400 |  0200);
        if (rc != SSH_OK)
        {
            emit ssh_log_message("Can't open remote file: %s\n"+
                                 std::string(ssh_get_error(active_session)));
            return rc;
        }

        rc = ssh_scp_write(scp, blob, length);
        if (rc != SSH_OK)
        {
            emit ssh_log_message( "Can't write to remote file: %s\n"+
                                  std::string(ssh_get_error(active_session)));
            return rc;
        }
        //nothing went wrong
        emit ssh_log_message("Succesfully copied bitfile "+PLL_BITFILE+" to "+"/tmp/");
        return SSH_OK;}
    catch (...){
        emit ssh_log_message("Could not copy bitfile");
        return -1;
    }
}





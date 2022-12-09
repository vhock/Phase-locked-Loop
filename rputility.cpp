#include "rputility.h"



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


int RPUtility::connect(std::string ipAddress){
     emit new_message("connecting on thread");

    ssh_session rp_session = ssh_new();
    if (rp_session == NULL) {
        exit(-1);
    }
    ssh_options_set(rp_session, SSH_OPTIONS_HOST, ipAddress.c_str()); //red pitaya address, needs to be replaced by prompt later
    ssh_options_set(rp_session, SSH_OPTIONS_USER, "root");
    ssh_options_set(rp_session, SSH_OPTIONS_PASSWORD_AUTH, "root");
    ssh_options_set(rp_session, SSH_OPTIONS_LOG_VERBOSITY, "3");//



  int returnValue=   ssh_connect(rp_session);
  if (returnValue == SSH_OK) {
      emit new_message("Successfully connected to "+ipAddress+"...");
      emit connectionStateChanged(0);
  }
  else {
      emit new_message("Connection to "+ipAddress+" failed");
      return -1;//no need to continue code execution, TODO error handling
  }

  //TODO skipped for now, but has to be done..
//  int hostVerifcation = verify_knownhost(rp_session);//check if the host is known for security
//  if (hostVerifcation != 0) {
//      return -1; //verification failed
//  }

  active_session=rp_session; //copy construction, important because if ref to rp_session is used the thread works with undefined memory
  verify_knownhost();
  int auth=ssh_userauth_password(active_session,"root","roottotototot");
  if (auth != SSH_AUTH_SUCCESS)
  {
    emit new_message("Authentication failed:"+std::string(ssh_get_error(rp_session)));

    fprintf(stderr, "Error authenticating with password: %s\n",
            ssh_get_error(rp_session));
    ssh_disconnect(rp_session);
    ssh_free(rp_session);
    return -1;
  }
  std::thread monitorSessionThread(&RPUtility::monitorActiveSession,this);
  monitorSessionThread.detach();
  //Sleep(50000);
  return 0;
 // ssh_free(rp_session);
}

int RPUtility::disconnect(){
    if (active_session!=NULL){
        ssh_disconnect(active_session);
        return 0;
    }
    return -1;
}
//never launch me in an undetached thread
void RPUtility::monitorActiveSession(){
    while (true){
       int isConnected=0;
       ssh_channel channel;
       int rc;
       channel = ssh_channel_new(active_session);
       rc = ssh_channel_open_session(channel);
       if (rc == SSH_ERROR){
           isConnected=0;

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
                emit new_message("Connection lost:"+std::to_string(rc));
            }else if (connection_status==1){
                emit new_message("Connection established:"+std::to_string(rc));
            }
       }
       Sleep(5000); //checking on the connection every few seconds is enough

    }
}


void RPUtility::fireTestEvent(){
    emit new_message("signal from red pitaya test test");
}
RPUtility::RPUtility()
{

}

#include "rputility.h"


int RPUtility::sendCommand(std::string command,std::string &serverReply){
    int rc;
    char buffer[1];
    int nbytes;
    ssh_channel  channel = ssh_channel_new(active_session);
    std::string receive = "";


    int sessionOK=ssh_channel_open_session(channel);
     if (sessionOK == SSH_ERROR){
         return -1;
     }

    rc = channel_request_exec(channel, command.c_str());
    if (rc ==SSH_ERROR) {
        emit new_message(ssh_get_error(active_session));
        return -1;
    }
   //channel_read(channel, buffer, sizeof(buffer),0);
   emit new_message("Buffer content:");
   nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0); //TODO how about is_stderr=1?
      while (nbytes > 0)
      {
              receive.append(buffer, nbytes);
              nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
      }

      emit new_message(receive); // TODO this still contains a \n character


    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    serverReply=receive;

    return SSH_OK;
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


int RPUtility::connect(std::string ipAddress){
     emit new_message("Establishing connection to "+ipAddress);
    ssh_session rp_session = ssh_new();
    if (rp_session == NULL) {
        exit(-1);
    }
    ssh_options_set(rp_session, SSH_OPTIONS_HOST, ipAddress.c_str()); //red pitaya address, needs to be replaced by prompt later
    ssh_options_set(rp_session, SSH_OPTIONS_LOG_VERBOSITY, "3");//

  int returnValue=   ssh_connect(rp_session);
  if (returnValue == SSH_OK) {
      emit new_message("Successfully connected to "+ipAddress+"...");
  }
  else {
      emit new_message("Connection to "+ipAddress+" failed."+std::string(ssh_get_error(rp_session)));
      return -1;//no need to continue code execution, TODO error handling
  }

  //TODO skipped for now, but has to be done..
 // verify_knownhost(rp_session);

//int hostVerifcation = verify_knownhost(rp_session);//check if the host is known for security
//  if (hostVerifcation != 0) {
//      return -1; //verification failed
//  }

 //Authentication
  int auth=ssh_userauth_password(rp_session,"root","root");
  if (auth != SSH_AUTH_SUCCESS)
  {
    emit new_message("Authentication failed:"+std::string(ssh_get_error(rp_session)));
    ssh_disconnect(rp_session);
    ssh_free(rp_session);
    return -1;
  }else {
      emit new_message("Authentication successful:"+std::string(ssh_get_error(rp_session)));

  }

  active_session=rp_session; //copy construction, important because if ref to rp_session is used all other threads works with undefined memory
  emit connectionStateChanged(1);
 // std::thread monitorSessionThread(&RPUtility::monitorActiveSession,this);// launch a thread which monitors the active session
 // monitorSessionThread.detach();
  return 0;
}

int RPUtility::authenticate(ssh_session rp_session,std::string user,std::string password){
    int auth=ssh_userauth_password(rp_session,user.c_str(),password.c_str());
    if (auth != SSH_AUTH_SUCCESS)
    {
      emit new_message("Authentication failed:"+std::string(ssh_get_error(rp_session)));
      ssh_disconnect(rp_session);
      ssh_free(rp_session);
      return -1;
    }

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
          emit new_message("Disconnected");
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
       ssh_channel_set_blocking(channel,true);
       if (ssh_channel_is_closed(channel)==0){
return ;
       }
       rc = ssh_channel_open_session(channel);
       if (rc == SSH_ERROR){//connection lost
           isConnected=0;
           emit new_message(std::string(ssh_get_error(active_session)));

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

int RPUtility::scp_helloworld()
{
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
        emit new_message("Error initializing scp session: %s\n"+
                         std::string(ssh_get_error(active_session)));
      fprintf(stderr, "Error initializing scp session: %s\n",
              ssh_get_error(active_session));
      ssh_scp_free(scp);
      return rc;
    }
    //simple text file
   QFile qfile(":/testfile.txt");
   bool qFileExists=qfile.exists();
   qfile.open(QIODevice::ReadOnly);
   std::string fileContent= qfile.readAll().toStdString();
   fileContent.c_str();

   QFile knightRider(":/knight_rider.bit");
   bool kRxists=qfile.exists();
   knightRider.open(QIODevice::ReadOnly);
   QByteArray blob = knightRider.readAll();





  const char *helloworld =fileContent.c_str(); //"Hello, world!\n";
  int length = strlen(helloworld);
  rc = ssh_scp_push_directory(scp, "helloworld", 0200);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Can't create remote directory: %s\n",
            ssh_get_error(active_session));
    return rc;
  }

//  rc = ssh_scp_push_file
//    (scp, "helloworld.txt", length, 0400 |  0200);
//  if (rc != SSH_OK)
//  {
//    fprintf(stderr, "Can't open remote file: %s\n",
//            ssh_get_error(active_session));
//    return rc;
//  }

//  rc = ssh_scp_write(scp, helloworld, length);
//  if (rc != SSH_OK)
//  {
//    fprintf(stderr, "Can't write to remote file: %s\n",
//            ssh_get_error(active_session));
//    return rc;
//  }



  length=blob.length();

  //knight rider
  rc = ssh_scp_push_file
    (scp, "knight_rider_copied.bit", length, 0400 |  0200);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Can't open remote file: %s\n",
            ssh_get_error(active_session));
    return rc;
  }

  rc = ssh_scp_write(scp, blob, length);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Can't write to remote file: %s\n",
            ssh_get_error(active_session));
    return rc;
  }


  return SSH_OK;
}


int interactive_shell_session(ssh_channel channel)
{
  /* Session and terminal initialization skipped */

 int rc=0;
  char buffer[256];
  int nbytes, nwritten;

  while (ssh_channel_is_open(channel) &&
         !ssh_channel_is_eof(channel))
  {
    nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 0);
    if (nbytes < 0) return SSH_ERROR;
    if (nbytes > 0)
    {
       // fwrite(buffer, 1, rc, stdout)
      nwritten = fwrite(buffer, 1, rc, stdout);
      if (nwritten != nbytes) return SSH_ERROR;

    if (!_kbhit())
    {
      Sleep(50000L); // 0.05 second
      continue;
    }


    nbytes = fread( buffer, sizeof(char), 10, stdin);
    if (nbytes < 0) return SSH_ERROR;
    if (nbytes > 0)
    {
      nwritten = ssh_channel_write(channel, buffer, nbytes);
      if (nwritten != nbytes) return SSH_ERROR;
    }
  }

  return rc;
}
}

RPUtility::RPUtility()
{

}

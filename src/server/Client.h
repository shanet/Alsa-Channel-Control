// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <openssl/evp.h>
#include <string>

#include "Constants.h"
#include "string.h"
#include "Crypto.h"

using namespace std;

#ifndef CLIENT_H
#define CLIENT_H

class Client {

public:
   Client(int socket, sockaddr_storage clientInfo, int id);

   Client();

   ~Client();

   int initCrypto();

   int send(string data, int useEnc=0);

   int receive(string *reply, int useEnc=0);

   void close();

   string getIPAddress();

   int getSocket();

   sockaddr_storage getClientInfo();

   int getID();

   // Crypto handshake functions
   int sendLocalPubKey();

   int receiveRemotePubKey();

   int sendAESKey();

private:
   int id;
   int socket;
   sockaddr_storage clientInfo;
   Crypto *crypto;
};

#endif

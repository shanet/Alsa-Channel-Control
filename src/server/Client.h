// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control
#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include "string.h"

#ifndef CONSTANTS_H
#include "Constants.h"
#endif
#ifndef CRYPT_H
#include "Crypt.h"
#endif

extern int verbose;
extern char *prog;

class Client {

public:
   Client(int socket, sockaddr_storage clientInfo, int id, Keypair *keypair);

   Client();

   ~Client();

   int send(std::string data);

   int receive(std::string *reply);

   void close();

   std::string getIPAddress();

   int getSocket();

   sockaddr_storage getClientInfo();

   int getID();

private:
   int id;
	int socket;
	sockaddr_storage clientInfo;
   Keypair *keypair;
};

#endif

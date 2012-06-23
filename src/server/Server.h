// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#ifndef SERVER_H
#define SERVER_H

#include <sstream>
#include <stdio.h>

#include "Client.h"

#ifndef CONSTANTS_H
#include "Constants.h"
#endif
#ifndef CRYPT_H
#include "Crypt.h"
#endif

// Defaults
#define DEFAULT_BACKLOG 10

// Error codes
#define FAILED_TO_BIND 2
#define FAILED_TO_LISTEN 3
#define FAILED_TO_GET_ADDR_INFO 4
#define FAILED_TO_GEN_KEY 5

extern int verbose;
extern char *prog;

class Server {

public:
   Server(int port, int backlog=DEFAULT_BACKLOG);

   Server();

   ~Server();

   int start(int aiFamily=AF_UNSPEC, int aiFlags=AI_PASSIVE);

   Client acceptConnection();

   void stop();

   void setPort(int port);

   int getListSock() const;

   int getPort() const;

   gcry_sexp_t getPublicKey();

   int setPublicKey(char *pubkey);

   int setPrivateKey(char *prikey);

private:
   int numClients;
   int port;
   int backlog;
   int listSock;
   bool isRunning;
   addrinfo *serverInfo;
   Keypair *keypair;

   void init();

   int getAddressInfo(int aiFamily, int aiFlags);

   int bind();
};

#endif

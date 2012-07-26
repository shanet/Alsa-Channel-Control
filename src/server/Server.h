// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include <sstream>

#include "Client.h"
#include "Crypto.h"
#include "Constants.h"

// Defaults
#define DEFAULT_BACKLOG 10

// Error codes
#define FAILED_TO_BIND 2
#define FAILED_TO_LISTEN 3
#define FAILED_TO_GET_ADDR_INFO 4

using namespace std;

#ifndef SERVER_H
#define SERVER_H

class Server {

public:
   Server(int port, int useCrypto=0, int backlog=DEFAULT_BACKLOG);

   Server();

   ~Server();

   int start(int aiFamily=AF_UNSPEC, int aiFlags=AI_PASSIVE);

   Client acceptConnection();

   void stop();

   void setPort(int port);

   int getListSock() const;

   int getPort() const;

   int isStarted() const;

private:
   int numClients;
   int port;
   int backlog;
   int listSock;
   int mIsStarted;
   addrinfo *serverInfo;
   int useCrypto;
   Crypto *crypto;

   int getAddressInfo(int aiFamily, int aiFlags);

   int bind();
};

#endif

// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <string>
#include "string.h"

#define SUCCESS 0
#define FAILURE -1
#define BUFFER 1000

using namespace std;

#ifndef CLIENT_H
#define CLIENT_H

class Client {

public:
   Client(int socket, sockaddr_storage clientInfo, int id);

   Client();

   ~Client();

   int send(string data);

   int receive(string *reply);

   void close();

   string getIPAddress();

   int getSocket();

   sockaddr_storage getClientInfo();

   int getID();

private:
   int id;
	int socket;
	sockaddr_storage clientInfo;
};

#endif

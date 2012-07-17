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
#include <string.h>
#include <sstream>

#include "Constants.h"
#include "Crypto.h"

// Error codes
#define FAILED_TO_CONNECT       2
#define FAILED_TO_LISTEN        3
#define FAILED_TO_GET_ADDR_INFO 4

using namespace std;

class Client {
	
public:
   Client(string host, int port);

   ~Client();

   int connectToServer(int aiFamily=AF_UNSPEC);
   
   int send(string data, int useEnc=0);

   int receive(string *reply, int useEnc=0);

   void closeConnection();

   string getServerIPAddress() const;

   int getConnSock() const;

   int getPort() const;

   void setPort(int port);

   void setHost(string host);

   // Crypto handshake functions
   int sendLocalPubKey();

   int receiveRemotePubKey();

   int sendAESKey();

   int receiveAESKey();

private:
   string host;
   int port;
   int connSock;
   addrinfo *serverInfo;
   Crypto *crypto;

   int getAddressInfo(int aiFamily=AF_UNSPEC);
   
   int connect();
};
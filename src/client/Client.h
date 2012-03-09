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

#define SUCCESS 0
#define FAILURE -1
#define BUFFER 1000

// Error codes
#define FAILED_TO_CONNECT 2
#define FAILED_TO_LISTEN 3
#define FAILED_TO_GET_ADDR_INFO 4

using namespace std;

class Client {
	
public:
   Client(string host, int port);

   ~Client();

   int connectToServer(int aiFamily=AF_UNSPEC);
   
   int send(string data);

   int receive(string *reply);

   void closeConnection();

   string getServerIPAddress() const;

   int getConnSock() const;

   int getPort() const;

   void setPort(int port);

   void setHost(string host);

private:
   string host;
   int port;
   int connSock;
   addrinfo *serverInfo;

   int getAddressInfo(int aiFamily=AF_UNSPEC);
   
   int connect();
};
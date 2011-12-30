#include "client.h"

#define NORMAL_EXIT 0

using namespace std;

int main(int argc, char *argv[]) {

   if(argc < 3) {
      cout << "Usage: client [hostname] [port]" << endl;
      return NORMAL_EXIT;
   }

   Client client((string)argv[1], atoi(argv[2]));
   
   // Connect to server
   if(client.connectToServer() != SUCCESS) {
      cout << "Error connecting to server." << endl;
      return FAILURE;
   }

   // Confirm connection
   cout << "Connected to " << client.getServerIPAddress() << endl;

   
   string msg, reply;

   while(1) {

      int recvLen = client.receive(&reply);
      
      if(recvLen == FAILURE)
         cout << "Communication error with server. Non-fatal." << endl;
      else if(recvLen == 0) {
         cout << "Server closed connection." << endl;
         break;
      }

      cout << "Server sent: " << reply << endl;

      cout << "Send to server: ";
      getline(cin, msg);

      if(msg.compare("end") == 0)
         break;

      if(client.send(msg) == FAILURE)
         cout << "Error sending data to server. Non-fatal.";

      cout << "Waiting for reply..." << endl;
   }

   cout << "Closing connection." << endl;
   client.closeConnection();

   return NORMAL_EXIT;
}

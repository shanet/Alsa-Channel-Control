#include "client.h"

int main(int argc, char *argv[]) {

   bool isVerbose = false;
   string server = DEFAULT_CLIENT;
   int port = DEFAULT_PORT;
   vector<string> channels, vols;

   // If no arguments are given, assume client is localhost
   if(argc > 1) {
      // Parse the remaining command line args
      for(unsigned short i=1; i<argc; i++) {
         if(strcmp(argv[1], "--server") == 0 || strcmp(argv[1], "-s") == 0) {
            // Make sure we're still in bounds
            if(i+1 < argc) {
               server = (string)argv[1];
            } else {
               cout  << "Server argument given without valid server" << endl;
               return FAILURE;
            }

         } else if(strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {
            // Make sure we're still in bounds
            if(i+1 < argc) {
               // Convert the given port to an int
               port = atoi(argv[i+1]);
            } else {
               cout  << "Port argument given without valid port" << endl;
               return FAILURE;
            }

         } else if(strcmp(argv[i], "--channel") == 0 || strcmp(argv[i], "-c") == 0) {
            // Make sure we're still in bounds
            if(i+2 < argc) {
               // Convert the given port to an int
               channels.push_back(argv[i+1]);
               vols.push_back(argv[i+2]);
            } else if(i+1 < argc) {
               cout  << "Channel argument given without valid volumes" << endl;
               return FAILURE;
            } else {
               cout  << "Channel argument given without valid channel" << endl;
               return FAILURE;
            }

         // Check for verbose
         } else if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-V") == 0) {
            isVerbose = true;
         // Check for version
         } else if(strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printVersion();
            return NORMAL_EXIT;
         // Check for help
         } else if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printUsage();
            return NORMAL_EXIT;
         }
      }
   }

   // Init the client object
   cout << server << " " << port << endl;
   Client client(server, port);

   // Connect to server
   if(client.connectToServer() != SUCCESS) {
      cout << "Error connecting to server" << endl;
      return FAILURE;
   }

   // Confirm connection
   cout << "Connected to " << client.getServerIPAddress() << endl;

   stringstream command;
   string reply;

   // Get initial welcome from server
   client.receive(&reply);

   if(reply.compare("helo\n") == 0) {
      // Send as many commands as there are in the channels vector
      for(unsigned int i=0; i<channels.size(); i++) {
         // Construct the command to be sent
         command << channels[i] << "=" << vols[i];

         if(isVerbose) {
            cout << "Sending \"" << command.str() << "\" to server" << endl;
         }

         // Send the command to the server
         if(client.send(command.str()) == FAILURE) {
            cout << "Error sending data to server";
         }

         if(isVerbose) {
            cout << "Waiting for reply from server..." << endl;
         }

         // Get server reply
         int recvLen = client.receive(&reply);

         if(recvLen == FAILURE) {
            if(isVerbose) {
               cout << "Communication error with server. Non-fatal." << endl;
            }
            continue;
         // If no data was received, the server closed the connection
         } else if(recvLen == 0) {
            cout << "Server unexpectedly closed connection" << endl;
            break;
         // Did it work?!
         } else if(reply.compare("ok\n") == 0) {
            if(isVerbose) {
               cout << "Volume set successfully" << endl;
            }
         // Check if there was something wrong with setting the volume
         } else if(reply.compare("err\n") == 0) {
            cout << "Error setting volume with command \"" << command.str() << "\"" << endl;
         // Check if server requested to end connection
         } else if(reply.compare("end\n") == 0) {
            if(isVerbose) {
               cout << "Server requested to close connection" << endl;
            }
            break;
         }
      }
   } else {
      if(isVerbose) {
         cout << "Server did not send proper handshake" << endl;
      }
   }

   cout << "Closing connection with server" << endl;
   client.send("end");
   client.closeConnection();

   return NORMAL_EXIT;
}


void printUsage() {
   cout << "Usage: " << NAME << " --server [server] --port [port] [options] --channel [channel] [leftVol],[rightVol]\n"
        << "\t--server\t(-s)\tThe server to connect to (localhost if omitted)\n"
        << "\t--port\t(-p)\tPort to connect on (" << DEFAULT_PORT << " if omitted)\n"
        << "\t--channel\t(-c)\tThe Alsa channel to change\n"
        << "\t--verbose\t(-V)\tIncrease verbosity\n"
        << "\t--version\t(-v)\tPrint version\n"
        << "\t--help\t\t(-h)\tDisplay this message"
        << endl;
}


void printVersion() {
   cout << VERSION << endl;
}
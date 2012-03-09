// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include "client.h"

int main(int argc, char *argv[]) {

   string server = DEFAULT_CLIENT;    // Host to connect to
   int port = DEFAULT_PORT;           // Port to connect to server on
   vector<string> channels, vols;     // The given channels and volumes that will be sent to the server
   char c;                            // Char for processing command line args
   int optIndex;                      // Index of long opts for processing command line args

   // Set the program name we're running under and default verbose level
   prog = argv[0];
   verbose = NO_VERBOSE;

   // Valid long options
   static struct option longOpts[] = {
      {"host", required_argument, NULL, 'H'},
      {"port", required_argument, NULL, 'p'},
      {"channel", required_argument, NULL, 'c'},
      {"volume", required_argument, NULL, 'v'},
      {"verbose", no_argument, NULL , 'V'},
      {"version", no_argument, NULL, 'q'},
      {"help", no_argument, NULL, 'h'}
   };

   // Parse the command line args
   while((c = getopt_long(argc, argv, "hVH:p:c:v:", longOpts, &optIndex)) != -1) {
      switch(c) {
         // Print help
         case 'h':
            printUsage();
            return NORMAL_EXIT;
         // Print version
         case 'q':
            printVersion();
            return NORMAL_EXIT;
         // Set verbose level
         case 'V':
            verbose++;
            break;
         // The host to connect to
         case 'H':
            server = optarg;
         // The port to start the server on
         case 'p':
            port = atoi(optarg);
            break;
         // Alsa channel to change
         case 'c':
            channels.push_back(optarg);
            break;
         // The volume to set
         case 'v':
            vols.push_back(optarg);
            break;
         // Invalid option
         case '?':
         default:
            fprintf(stderr, "%s: Try \"%s -h\" for usage information.\n", prog, prog);
            return ABNORMAL_EXIT;
      }
   }


   // Check that there are equal number of channels and volumes
   if(channels.size() != vols.size()) {
      fprintf(stderr, "%s: Invalid number of channels for number of volumes given\n", prog);
      return ABNORMAL_EXIT;
   }

   // Init the client object
   //printf("%s: server << " " << port << endl;
   Client client(server, port);

   // Connect to server
   if(client.connectToServer() != SUCCESS) {
      fprintf(stderr, "%s: Error connecting to server\n", prog);
      return ABNORMAL_EXIT;
   }

   // Confirm connection
   if(verbose >= VERBOSE) {
      printf("%s: Connected to %s\n", prog, client.getServerIPAddress().c_str());
   }

   stringstream command;
   string reply;

   try {
      // Get initial welcome from server
      client.receive(&reply);
      if(reply.compare("helo\n") != 0) {
         throw FAILURE;
      }

      // Complete handshake
      if(client.send("helo\n") == FAILURE) {
         throw FAILURE;
      }

      // Wait for the ready command to start sending volume commands
      client.receive(&reply);
      if(reply.compare("redy\n") != 0) {
         throw FAILURE;
      }

      // Send as many commands as there are in the channels vector
      for(unsigned int i=0; i<channels.size(); i++) {
         // Construct the command to be sent (clear the stringstream first though)
         command.str("");
         command << channels[i] << "=" << vols[i];

         if(verbose >= VERBOSE) {
            printf("%s: Sending \"%s\" to server\n", prog, command.str().c_str());
         }

         // Send the command to the server
         if(client.send(command.str()) == FAILURE) {
            fprintf(stderr, "%s: Error sending data to server\n", prog);
            throw FAILURE;
         }

         if(verbose >= VERBOSE) {
            printf("%s: Waiting for reply from server...\n", prog);
         }

         // Get server reply
         int recvLen = client.receive(&reply);

         if(recvLen == FAILURE) {
            if(verbose >= VERBOSE) {
               printf("%s: Communication error with server. Non-fatal.\n", prog);
            }
            throw FAILURE;
         // If no data was received, the server closed the connection
         } else if(recvLen == 0) {
            printf("%s: Server unexpectedly closed connection\n", prog);
            throw FAILURE;
         // Did it work?!
         } else if(reply.compare("ok\n") == 0) {
            if(verbose >= VERBOSE) {
               printf("%s: Volume set successfully\n", prog);
            }
         // Check if there was something wrong with setting the volume
         } else if(reply.compare("err\n") == 0) {
            fprintf(stderr, "%s: Error setting volume with command \"%s\"\n", prog, command.str().c_str());
            throw FAILURE;
         // Check if server requested to end connection
         } else if(reply.compare("end\n") == 0) {
            if(verbose >= VERBOSE) {
               printf("%s: Server requested to close connection\n", prog);
            }
            throw FAILURE;
         }
      }
   } catch (int exception) {
      if(verbose >= VERBOSE) {
         fprintf(stderr, "%s: Server did not send proper handshake\n", prog);
      }
   }

   if(verbose >= VERBOSE) {
      printf("%s: Closing connection with server\n", prog);
   }

   client.send("end");
   client.closeConnection();

   return NORMAL_EXIT;
}


void printUsage() {
   printf("%s: Usage: %s --host [host] --port [port] [options] --channel [channel] --volume [leftVol],[rightVol]\n\
          --host\t(-H)\tThe server to connect to (localhost if omitted)\n\
          --port\t(-p)\tPort to connect on (%d if omitted)\n\
          --channel\t(-c)\tThe Alsa channel to change\n\
          --volume\t(-v)\tThe volume to set (0-100%%)\n\
          --verbose\t(-V)\tIncrease verbosity\n\
          --version\t\tPrint version\n\
          --help\t\t(-h)\tDisplay this message\n", prog, prog, DEFAULT_PORT);
}


void printVersion() {
   printf("%s: %s\n", prog, VERSION);
}
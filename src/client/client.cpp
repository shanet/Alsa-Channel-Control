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
   useEnc = 0;
   verbose = NO_VERBOSE;

   // Valid long options
   static struct option longOpts[] = {
      {"host",    required_argument, NULL, 'H'},
      {"port",    required_argument, NULL, 'p'},
      {"encrypt", no_argument,       NULL, 'e'},
      {"channel", required_argument, NULL, 'c'},
      {"volume",  required_argument, NULL, 'v'},
      {"verbose", no_argument,       NULL, 'V'},
      {"version", no_argument,       NULL, 'q'},
      {"help",    no_argument,       NULL, 'h'}
   };

   // Parse the command line args
   while((c = getopt_long(argc, argv, "Hp:ec:v:Vqh", longOpts, &optIndex)) != -1) {
      switch(c) {
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
         // Set verbose level
         case 'V':
            verbose++;
            break;
         // Print version
         case 'q':
            printVersion();
            return NORMAL_EXIT;
         // Print help
         case 'h':
            printUsage();
            return NORMAL_EXIT;
         // Invalid option
         case '?':
         default:
            fprintf(stderr, "%s: Try \"%s -h\" for usage information.\n", prog, prog);
            return ABNORMAL_EXIT;
      }
   }


   // Check that there are equal number of channels and volumes
   if(channels.size() != vols.size()) {
      fprintf(stderr, "%s: Invalid number of channels for number of volumes given. Don't confuse '-v' with '-V'. Use \'%s -h\' for usage.\n", prog, prog);
      return ABNORMAL_EXIT;
   } else if(channels.size() == 0 || vols.size() == 0) {
      fprintf(stderr, "%s: No channels or volumes specified. Use \'%s -h\' for usage.\n", prog, prog);
      return ABNORMAL_EXIT;
   }

   // Init the client object
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
      // Do the handshake with the server
      int handshakeStatus = serverHandshake();
      if(handshakeStatus == FAILURE) {
         throw FAILURE;
      } else if(handshakeStatus == END) {
         throw END;
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
            if(verbose >= DBL_VERBOSE) {
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
   } catch (int exception) {}

   if(verbose >= VERBOSE) {
      printf("%s: Closing connection with server\n", prog);
   }

   // End the connection gracefully
   client.send("end\n", useEnc);

   // Check for end confirmation
   client.receive(&reply, useEnc);
   if(reply.compare("bye\n") != 0) {
      // If we didn't get it, oh well.
      if(verbose >= DBL_VERBOSE) {
         fprintf(stderr, "%s: Server failed to send end connection confirmation\n", prog);
      }
   }

   // Close it.
   client.closeConnection();

   return NORMAL_EXIT;
}


int serverHandshake() {
   string reply;
   int recvLen;

   // Print that we're connected
   if(verbose >= VERBOSE) {
      printf("%s: Connected to server... starting handshake...\n", prog);
   }

   // Get initial welcome from server
   client.receive(&reply, 0);

   // Validate the reply
   if(recvLen == FAILURE) {
      fprintf(stderr, "%s: Server failed to give proper handshake\n", prog);
      return FAILURE;
   } else if(recvLen == 0) {
      fprintf(stderr, "%s: Server unexpectedly closed connection\n", prog);
      return FAILURE;
   } else if(reply.compare("helo\n") != 0) {
      fprintf(stderr, "%s: Server did not send proper handshake\n", prog);
      return FAILURE;
   } else if(reply.compare("end\n") == 0) {
      return END;
   }

   // Tell the server if we're using encryption or not
   if(client.send(((useEnc) ? "enc\n" : "noenc\n"), 0) == FAILURE) {
      if(verbose >= VERBOSE) {
         fprintf(stderr, "%s: Error sending encryption type to server\n", prog);
      }
      return FAILURE;
   }

   // If using encryption, exchange keys
   if(useEnc) {
      // Get the server's pub key
      if(client.receiveRemotePubKey() == FAILURE) {
         return FAILURE;
      }

      // Send our public key
      if(client.sendLocalPubKey() == FAILURE) {
         return FAILURE;
      }

      // Get the AES key
      if(client.receiveAESKey() == FAILURE) {
         return FAILURE;
      }

      // Send a redy command to let the server know we're good to go (this is the last unecrypted message)
      if(client.send("redy\n", 0) == FAILURE) {
         if(verbose >= VERBOSE) {
            fprintf(stderr, "%s: Error completing handshake with server\n", prog);
         }
         return FAILURE;
      }
   }

   // Not using encryption
   } else {
      // Wait for the redy confirmation from the server
      recvLen = client.receive(&reply, 0);

      // Validate the reply
      if(recvLen == FAILURE) {
         fprintf(stderr, "%s: Server failed to give proper handshake\n", prog);
         return FAILURE;
      } else if(recvLen == 0) {
         fprintf(stderr, "%s: Server unexpectedly closed connection\n", prog);
         return FAILURE;
      } else if(reply.compare("end\n") == 0) {
         return END;
      } else if(reply.compare("redy\n") != 0) {
         return FAILURE;
      }
   }

   if(verbose >= VERBOSE) {
      printf("%s: Completed handshake with server\n", prog);
   }

   return SUCCESS;
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
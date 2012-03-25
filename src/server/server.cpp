// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include "server.h"

int main(int argc, char* argv[]) {
   int port = DEFAULT_PORT;   // The port the server is running on
   int pid;                   // Pid for forking on startup
   char c;                    // Char for processing command line args
   int optIndex;              // Index of long opts for processing command line args

   // Set the program name we're running under and default verbose level
   prog = argv[0];
   verbose = NO_VERBOSE;

   // Handle SIGINT's and SIGTERM's
   struct sigaction sa;
   sa.sa_handler = sigHandler;
   sa.sa_flags = SA_RESTART;
   sigemptyset(&sa.sa_mask);
   if(sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
      fprintf(stderr, "%s: Failed to install signal handlers\n", prog);
      return ABNORMAL_EXIT;
   }

   // Valid long options
   static struct option longOpts[] = {
      {"port", required_argument, NULL, 'p'},
      {"verbose", no_argument, NULL , 'v'},
      {"version", no_argument, NULL, 'V'},
      {"help", no_argument, NULL, 'h'}
   };

   // Parse the command line args
   while((c = getopt_long(argc, argv, "hvVp:", longOpts, &optIndex)) != -1) {
      switch(c) {
         // Print help
         case 'h':
            printUsage();
            return NORMAL_EXIT;
         // Print version
         case 'V':
            printVersion();
            return NORMAL_EXIT;
         // Set verbose level
         case 'v':
            verbose++;
            break;
         // The port to start the server on
         case 'p':
            port = atoi(optarg);
            break;
         case '?':
            fprintf(stderr, "%s: Try \"%s -h\" for usage information.\n", prog, prog);
         default:
            return ABNORMAL_EXIT;
      }
   }

   server.setPort(port);

   // Start the server
   printf("%s: %s version %s starting...\n", prog, NAME, VERSION);
   if(server.start() != SUCCESS) {
      fprintf(stderr, "%s: Failed to start server\n", prog);
      return FAILURE;
   }

   printf("%s: Server started on port %d\n", prog, server.getPort());

   // This is a server. Fork and return control to whatever started us
   if((pid = fork()) == -1) {
      fprintf(stderr, "%s: Failed to fork on startup\n", prog);
      return ABNORMAL_EXIT;
   } else if(pid != 0) {
      return NORMAL_EXIT;
   }

   // Wait for connections
   while(1) {
      // Accept any incoming connections
      client = server.acceptConnection();

      if(client.getSocket() == FAILURE) {
         fprintf(stderr, "%s: Got connection, but failed to accept", prog);
         continue;
      }

      // Fork the connection
      int pid;
      if(!(pid = fork())) {
         try {
            // Get hostname of connection
            printf("%s: Got connection from %s\n", prog, client.getIPAddress().c_str());

            string send, reply;

            // Send welcome
            if(verbose >= VERBOSE) {
               printf("%s: Sending helo to client %d\n", prog, client.getID());
            }
            // The usual defense for the use of a goto: It's there to avoid putting more logic in to avoid the loop below
            // if the client didn't send a proper handshake
            if(client.send("helo\n") == FAILURE) {
               if(verbose >= VERBOSE) {
                  fprintf(stderr, "%s: Error sending welcome to client %d\n", prog, client.getID());
               }
               throw FAILURE;
            }

            // Get reply welcome
            int recvLen = client.receive(&reply);

            // Validate the reply to complete the handshake
            if(recvLen == FAILURE) {
               fprintf(stderr, "%s: Client %d failed to give proper handshake\n", prog, client.getID());
               throw FAILURE;
            } else if(recvLen == 0) {
               fprintf(stderr, "%s: Client %d unexpectedly closed connection\n", prog, client.getID());
               throw FAILURE;
            } else if(reply.compare("helo\n") != 0) {
               fprintf(stderr, "%s: Client %d failed to give proper handshake\n", prog, client.getID());
               throw FAILURE;
            }

            // Tell the client we're ready for volume commands
            if(client.send("redy\n") == FAILURE) {
               if(verbose >= VERBOSE) {
                  fprintf(stderr, "%s: Error sending data to client %d\n", prog, client.getID());
               }
            } else {
               if(verbose >= VERBOSE) {
                  printf("%s: Client %d given ready command to start issuing volume changes\n", prog, client.getID());
               }
            }

            while(1) {
               // Get the reply from the client
               recvLen = client.receive(&reply);

               // Validate the reply
               if(recvLen == FAILURE) {
                  fprintf(stderr, "%s: Error receiving data from client %d\n", prog, client.getID());
               } else if(recvLen == 0) {
                  fprintf(stderr, "%s: Client %d unexpectedly closed connection\n", prog, client.getID());
                  break;
               }

               if(verbose >= VERBOSE) {
                  printf("%s: Reply from client %d: %s\n", prog, client.getID(), prog);
               }

               // Check if requesting to end connection
               if(reply.compare("end\n") == 0) {
                  if(verbose >= VERBOSE) {
                     printf("%s: client %d requested to close connection\n", prog, client.getID());
                  }

                  // Send an end confirmation
                  client.send("end\n");

                  // Use this is a way to exit the loop and close the connection gracefully
                  throw FAILURE;

               // Otherwise, treat reply as a change volume command
               } else if(parseCommand(reply) == SUCCESS) {
                  if(verbose >= VERBOSE) {
                     printf("%s: client %d: volume change successful\n", prog, client.getID());
                  }
                  send = "ok\n";
               // Anything else is an error
               } else {
                  if(verbose >= VERBOSE) {
                     fprintf(stderr, "%s: client %d: volume change unsuccessful\n", prog, client.getID());
                  }
                  send = "err\n";
               }

               // Send the response
               if(client.send(send) == FAILURE) {
                  if(verbose >= VERBOSE) {
                     fprintf(stderr, "%s: Error sending data to client %d\n", prog, client.getID());
                  }
                  throw FAILURE;
               } else {
                  if(verbose >= VERBOSE) {
                     printf("%s: Volume change confirmation sent to client %d\n", prog, client.getID());
                  }
               }
            }

         } catch (int e) {}

         printf("%s: Closing connection with client %d\n", prog, client.getID());
         client.close();

         // Kill this child
         exit(NORMAL_EXIT);
      } else {
         // Add the new child to the children list
         children.push_back(pid);
      }
   }

   // Wait for the kids to come home
   int childStatus;
   for(unsigned int i=0; i<children.size(); i++) {
      if(waitpid(children[i], &childStatus, 0) != -1) {
         children.erase(children.begin());
      }
   }

   // Shut it down
   printf("%s: Server stopping...", prog);
   server.stop();

   return NORMAL_EXIT;
}


int parseCommand(const string command) {
   // Command should be in the format [channel]=[leftVol],[rightVol]
   string channel;
   int equalsIndex, commaIndex;
   int leftVol, rightVol;

   // Get the equals and comma index
   if((equalsIndex = command.find('=')) == string::npos || (commaIndex = command.find(',')) == string::npos) {
      return PARSE_ERROR;
   }

   channel = command.substr(0, equalsIndex);
   leftVol = atoi(command.substr(equalsIndex+1, commaIndex).c_str());
   rightVol = atoi(command.substr(commaIndex+1).c_str());

   // Finally, change the volume!
   return changeVolume(channel, leftVol, rightVol);
}


void sigHandler(const int signal) {
   // Clean up and exit if requested by the system
   if(signal == SIGINT || signal == SIGTERM) {
      if(verbose >= DBL_VERBOSE) {
         printf("%s: Cleaning up...\n", prog);
      }

      // Kill the children
      for(unsigned int i=0; i<children.size(); i++) {
         kill(children[i], SIGTERM);
      }

      // Stop the server and exit
      server.stop();
      exit(NORMAL_EXIT);
   }
}


int changeVolume(const string channel, const int leftVolume, const int rightVolume) {
   const char *alsaBinary = "amixer";
   stringstream vol;

   // Construct the volume string
   vol << leftVolume << "%," << rightVolume << "%";

   if(verbose >= VERBOSE) {
      printf("%s: Setting volume to: %s %s\n", prog, channel.c_str(), vol.str().c_str());
   }

   // Fork and exec the Alsa binary to change the volume
   int pid;
   int status = -1;
   if((pid = fork()) == -1) {
      fprintf(stderr, "%s: Failed to create child\n", prog);
   } else if(pid == 0) {
      // Close stdout and stderr to supress output from the Alsa binary unless >= double verbose is specified
      if(verbose < DBL_VERBOSE) {
         fclose(stdout);
         fclose(stderr);
      }

      // Execute the alsa binary to change the volume
      if(execlp(alsaBinary, alsaBinary, "sset", channel.c_str(), vol.str().c_str(), NULL) == -1) {
         fprintf(stderr, "%s: Failed to execute \"%s\": %s\n", prog, alsaBinary, strerror(errno));
      }
   } else {
      // Wait for the fork to exit and return its exit code
      waitpid(pid, &status, 0);

      if(verbose >= DBL_VERBOSE) {
         printf("%s: %s exited with exit code %d\n", prog, alsaBinary, status);
      }

      return status;
   }

   // How did you get here?
   return -1;
}


void printUsage() {
   printf("%s: Usage: %s [options]\n\
          --port\t(-p) [port]\tSpecify port number\n\
          --verbose\t(-v)\t\tIncrease verbosity up to three levels\n\
          --version\t(-V)\t\tPrint version\n\
          --help\t(-h)\t\tDisplay this message\n", prog, prog);
}


void printVersion() {
   printf("%s: %s\n", NAME, VERSION);
}

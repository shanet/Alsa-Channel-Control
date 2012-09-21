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
   int noFork = 0;            // To fork or not to fork, that is the function

   // Set the program name we're running under and default verbose level
   prog = argv[0];
   useEnc = 0;
   verbose = NO_VERBOSE;
   display = NULL;

   // Handle SIGINT's, SIGTERM's, and SIGCHLD's
   struct sigaction sa;
   sa.sa_handler = sigHandler;
   sa.sa_flags = SA_RESTART;
   sigemptyset(&sa.sa_mask);
   if(sigaction(SIGINT,  &sa, NULL) == -1 || 
      sigaction(SIGTERM, &sa, NULL) == -1 ||
      sigaction(SIGCHLD, &sa, NULL) == -1)
   {
      fprintf(stderr, "%s: Failed to install signal handlers\n", prog);
      return ABNORMAL_EXIT;
   }

   // Valid long options
   static struct option longOpts[] = {
      {"port",    required_argument, NULL, 'p'},
      {"encrypt", no_argument,       NULL, 'e'},
      {"no-fork", no_argument,       NULL, 'f'},
      {"verbose", no_argument,       NULL, 'v'},
      {"version", no_argument,       NULL, 'V'},
      {"help",    no_argument,       NULL, 'h'}
   };

   // Parse the command line args
   while((c = getopt_long(argc, argv, "p:efvVh", longOpts, &optIndex)) != -1) {
      switch(c) {
         // The port to start the server on
         case 'p':
            port = atoi(optarg);
            break;
         // Use encryption
         case 'e':
            useEnc = 1;
            break;
         case 'f':
            noFork = 1;
            break;
         // Set verbose level
         case 'v':
            verbose++;
            break;
         // Print version
         case 'V':
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

   // Init the server
   server = Server(port, useEnc);

   // Start the server
   printf("%s: %s version %s starting...\n", prog, NAME, VERSION);
   if(server.start() != SUCCESS) {
      fprintf(stderr, "%s: Failed to start server\n", prog);
      return FAILURE;
   }

   printf("%s: Server started on port %d\n", prog, server.getPort());

   // This is a server. Fork and return control to whatever started us
   if(!noFork) {
      if((pid = fork()) == -1) {
         fprintf(stderr, "%s: Failed to fork on startup\n", prog);
         return ABNORMAL_EXIT;
      } else if(pid != 0) {
         return NORMAL_EXIT;
      }
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
         int commandResult;
         try {
            // Get hostname of connection
            printf("%s: Got connection from %s\n", prog, client.getIPAddress().c_str());

            // Do the handshake with the client
            if(clientHandshake() == FAILURE) {
               throw FAILURE;
            }

            do {
               // Get and process command from the client
               commandResult = processCommand();
               if(commandResult == FAILURE) {
                  throw FAILURE;
               }
            } while(commandResult != END);

         } catch (int e) {
            // If there was a failure in the handshake or the command, send the end command to the client
            if(verbose >= DBL_VERBOSE) {
               fprintf(stderr, "%s: Client %d: Sending end command due to error\n", prog, client.getID());
            }
            if(client.send("end\n", useEnc) == FAILURE && verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending end command to client %d\n", prog, client.getID());
            }
         }

         printf("%s: Closing connection with client %d\n", prog, client.getID());
         client.close();

         // Kill this child
         exit(NORMAL_EXIT);
      } else {
         // Add the new child to the children list
         children.push_back(pid);
      }
   }

   // Wait for the remaining kids to come home
   int childStatus;
   for(list<int>::iterator iter = children.begin(); iter != children.end(); iter++) {
      if(waitpid(*iter, &childStatus, 0) != -1) {
         children.erase(iter);
      }
   }

   // Shut it down
   printf("%s: Server stopping...", prog);
   server.stop();

   // Disconnect from X
   if(display != NULL) {
      XCloseDisplay(display);
   }

   return NORMAL_EXIT;
}


int clientHandshake() {
   string reply;
   int recvLen;

   // Send helo
   if(verbose >= VERBOSE) {
      printf("%s: Sending helo to client %d\n", prog, client.getID());
   }

   if(client.send("helo\n") == FAILURE) {
      if(verbose >= VERBOSE) {
         fprintf(stderr, "%s: Error sending welcome to client %d\n", prog, client.getID());
      }
      return FAILURE;
   }

   // Reply to helo should be whether to use encryption or not
   recvLen = client.receive(&reply, useEnc);

   // Validate the reply
   if(recvLen == FAILURE) {
      fprintf(stderr, "%s: Client %d failed to give proper handshake\n", prog, client.getID());
      return FAILURE;
   } else if(recvLen == 0) {
      fprintf(stderr, "%s: Client %d unexpectedly closed connection\n", prog, client.getID());
      return FAILURE;
   } else if(reply.compare("enc\n") == 0) {
      clientEnc = 1;
   } else if(reply.compare("noenc\n") == 0) {
      clientEnc = 0;
   } else {
      fprintf(stderr, "%s: Client %d failed to give proper handshake\n", prog, client.getID());
      return FAILURE;
   }

   // If the client requested encryption, init crypto object in the client object and exchange keys
   if(clientEnc) {
      if(verbose >= DBL_VERBOSE) {
         printf("%s: Client %d: Initializing encryption functions\n", prog, client.getID());
      }
      client.initCrypto();

      if(verbose >= VERBOSE) {
         printf("%s: Client %d requested encryption. Exchanging keys...\n", prog, client.getID());
      }

      // Send our public key
      if(verbose >= DBL_VERBOSE) {
         printf("%s: Client %d: Sending local public key\n", prog, client.getID());
      }
      if(client.sendLocalPubKey() == FAILURE) {
         fprintf(stderr, "%s: Client %d: Failed to send local public key\n", prog, client.getID());
         return FAILURE;
      }

      // Get the client's public key or error of receipt of the server's public key
      if(verbose >= DBL_VERBOSE) {
         printf("%s: Client %d: Receiving remote public key\n", prog, client.getID());
      }
      if(client.receiveRemotePubKey() == FAILURE) {
         if(verbose >= VERBOSE) {
            fprintf(stderr, "%s: Client %d: Failed to receive remote public key\n", prog, client.getID());
         }
         // Let the client know of the failure
         client.send("err\n");
         return FAILURE;
      }

      // Send the AES key to the client (the sendAESKey() function takes care of encrypting it first)
      if(verbose >= DBL_VERBOSE) {
         printf("%s: Client %d: Sending AES key\n", prog, client.getID());
      }
      if(client.sendAESKey() == FAILURE) {
         fprintf(stderr, "%s: Failed to send AES key to client %d\n", prog, client.getID());
         return FAILURE;
      }

      // Get the redy from the client that it is good to go
      recvLen = client.receive(&reply, 0);

      if(recvLen == FAILURE) {
         fprintf(stderr, "%s: Client %d: Failed to complete encryption handshake\n", prog, client.getID());
         return FAILURE;
      } else if(recvLen == 0) {
         fprintf(stderr, "%s: Client %d: Unexpectedly closed connection\n", prog, client.getID());
         return FAILURE;
      } else if(reply.compare("redy\n") != 0) {
         fprintf(stderr, "%s: Client %d: Failed encryption handshake\n", prog, client.getID());
         return FAILURE;
      }
   }

   // Send redy to tell the client the handshake is complete and we're ready to accept commands
   if(client.send("redy\n", useEnc) == FAILURE) {
      if(verbose >= VERBOSE) {
         fprintf(stderr, "%s: Error sending data to client %d\n", prog, client.getID());
         return FAILURE;
      }
   } else {
      if(verbose >= VERBOSE) {
         printf("%s: Client %d completed handshake\n", prog, client.getID());
      }
   }

   return SUCCESS;
}


int processCommand() {
   string reply;
   string send;
   int recvLen;
   int num_channels;

   // Get the command
   recvLen = client.receive(&reply, useEnc);

   // Validate the reply
   if(recvLen == FAILURE) {
      fprintf(stderr, "%s: Error receiving command data from client %d\n", prog, client.getID());
      return FAILURE;
   } else if(recvLen == 0) {
      fprintf(stderr, "%s: Client %d unexpectedly closed connection\n", prog, client.getID());
      return FAILURE;
   }

   // Print the command if verbose
   if(verbose >= VERBOSE) {
      printf("%s: Reply from client %d: %s\n", prog, client.getID(), reply.c_str());
   }

   // End command
   if(reply.compare("end\n") == 0) {
      if(verbose >= VERBOSE) {
         printf("%s: client %d requested to close connection\n", prog, client.getID());
      }

      // Send an end confirmation
      client.send("bye\n", useEnc);

      return END;

   // Play command
   } else if(reply.compare("play\n") == 0) {
      if(verbose >= VERBOSE) {
         printf("%s: Client %d issued play command\n", prog, client.getID());
      }

      // Send the play command
      if(sendMediaKey(XF86AudioPlay) != FAILURE) {
         // Send ok
         if(client.send("ok\n", useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending play command confirmation to client %d\n", prog, client.getID());
            }
            return FAILURE;
         }
         if(verbose >= DBL_VERBOSE) {
            printf("%s: Client %d: Play command successful\n", prog, client.getID());
         }
      } else {
         // Send err
         if(client.send("err\n", useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending play command error to client %d\n", prog, client.getID());
            }
            return FAILURE;
         }
         if(verbose >= DBL_VERBOSE) {
            printf("%s: Client %d: Play command error\n", prog, client.getID());
         }
         return FAILURE;
      }

      return SUCCESS;

   // Next command
   } else if(reply.compare("next\n") == 0) {
      if(verbose >= VERBOSE) {
         printf("%s: Client %d issued next command\n", prog, client.getID());
      }

      // Send the next command
      if(sendMediaKey(XF86AudioNext) != FAILURE) {
         // Send ok
         if(client.send("ok\n", useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending next command confirmation to client %d\n", prog, client.getID());
            }
            return FAILURE;
         }
         if(verbose >= DBL_VERBOSE) {
            printf("%s: Client %d: Next command successful\n", prog, client.getID());
         }
      } else {
         // Send err
         if(client.send("err\n", useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending next command error to client %d\n", prog, client.getID());
            }
            return FAILURE;
         }
         if(verbose >= DBL_VERBOSE) {
            printf("%s: Client %d: Next command error\n", prog, client.getID());
         }
         return FAILURE;
      }

      return SUCCESS;

   // Prev command
   } else if(reply.compare("prev\n") == 0) {
      if(verbose >= VERBOSE) {
         printf("%s: Client %d issued prev command\n", prog, client.getID());
      }

      // Send the prev command
      if(sendMediaKey(XF86AudioPrev) != FAILURE) {
         // Send ok
         if(client.send("prev\n", useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending prev command confirmation to client %d\n", prog, client.getID());
            }
            return FAILURE;
         }
         if(verbose >= DBL_VERBOSE) {
            printf("%s: Client %d: Prev command successful\n", prog, client.getID());
         }
      } else {
         // Send err
         if(client.send("err\n", useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending prev command error to client %d\n", prog, client.getID());
            }
            return FAILURE;
         }
         if(verbose >= DBL_VERBOSE) {
            printf("%s: Client %d: Prev command error\n", prog, client.getID());
         }
         return FAILURE;
      }

      return SUCCESS;

   // Stop command
   } else if(reply.compare("prev\n") == 0) {
      if(verbose >= VERBOSE) {
         printf("%s: Client %d issued stop command\n", prog, client.getID());
      }

      // Send the stop command
      if(sendMediaKey(XF86AudioStop) != FAILURE) {
         // Send ok
         if(client.send("stop\n", useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending stop command confirmation to client %d\n", prog, client.getID());
            }
            return FAILURE;
         }
         if(verbose >= DBL_VERBOSE) {
            printf("%s: Client %d: Stop command successful\n", prog, client.getID());
         }
      } else {
         // Send err
         if(client.send("err\n", useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending stop command error to client %d\n", prog, client.getID());
            }
            return FAILURE;
         }
         if(verbose >= DBL_VERBOSE) {
            printf("%s: Client %d: Stop command error\n", prog, client.getID());
         }
         return FAILURE;
      }

      return SUCCESS;

   // Volume command
   } else if(reply.compare("vol\n") == 0) {
      if(verbose >= VERBOSE) {
         printf("%s: Client %d issued volume command\n", prog, client.getID());
      }

      // Send ok
      if(verbose >= DBL_VERBOSE) {
         printf("%s: Client %d: Sending volume command ok confirmation to client\n", prog, client.getID());
      }
      if(client.send("ok\n", useEnc) == FAILURE) {
         if(verbose >= VERBOSE) {
            fprintf(stderr, "%s: Error sending volume command confirmation to client %d\n", prog, client.getID());
         }
         return FAILURE;
      }

      // Get number of channels being set
      recvLen = client.receive(&reply, useEnc);
      num_channels = atoi(reply.c_str());

      // Validate the reply
      if(recvLen == FAILURE) {
         fprintf(stderr, "%s: Error receiving command data from client %d\n", prog, client.getID());
         return FAILURE;
      } else if(recvLen == 0) {
         fprintf(stderr, "%s: Client %d unexpectedly closed connection\n", prog, client.getID());
         return FAILURE;
      }

      // Send ok
      if(verbose >= DBL_VERBOSE) {
         printf("%s: Client %d: Sending volume command channel number ok confirmation to client\n", prog, client.getID());
      }
      if(client.send("ok\n", useEnc) == FAILURE) {
         if(verbose >= VERBOSE) {
            fprintf(stderr, "%s: Error sending volume command channel number confirmation to client %d\n", prog, client.getID());
         }
         return FAILURE;
      }

      for(int i=0; i<num_channels; i++) {
         // Get volume command arg (channel and vol percent)
         recvLen = client.receive(&reply, useEnc);

         // Validate the reply
         if(recvLen == FAILURE) {
            fprintf(stderr, "%s: Error receiving command data from client %d\n", prog, client.getID());
            return FAILURE;
         } else if(recvLen == 0) {
            fprintf(stderr, "%s: Client %d unexpectedly closed connection\n", prog, client.getID());
            return FAILURE;
         }

         // Change the volume
         if(parseVolCommand(reply) == SUCCESS) {
            if(verbose >= VERBOSE) {
               printf("%s: client %d: volume command successful\n", prog, client.getID());
            }
            send = "ok\n";
         } else {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: client %d: volume command unsuccessful\n", prog, client.getID());
            }
            send = "err\n";
         }

         // Send the response
         if(client.send(send, useEnc) == FAILURE) {
            if(verbose >= VERBOSE) {
               fprintf(stderr, "%s: Error sending command reply to client %d\n", prog, client.getID());
            }
            return FAILURE;
         } else {
            if(verbose >= VERBOSE) {
               printf("%s: Volume change confirmation sent to client %d\n", prog, client.getID());
            }
         }
      }

      return SUCCESS;

   // Unknown command
   } else {
      fprintf(stderr, "%s: client %d sent unknown command\n", prog, client.getID());

      // Send an error to the client
      if(client.send("err\n", useEnc) == FAILURE) {
         if(verbose >= VERBOSE) {
            fprintf(stderr, "%s: Error sending command error to client %d\n", prog, client.getID());
         }
      }
      return FAILURE;
   }
}


int parseVolCommand(const string command) {
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
      for(list<int>::iterator iter = children.begin(); iter != children.end(); iter++) {
         kill(*iter, SIGTERM);
      }

      // Disconnect from X
      if(display != NULL) {
         XCloseDisplay(display);
      }

      // Stop the server and exit
      server.stop();
      exit(NORMAL_EXIT);
   // Remove a child from the list if it returned
   } else if(signal == SIGCHLD) {
      // Get the pid of the child that returned
      pid_t child = waitpid(-1, NULL, WNOHANG);
      if(child != -1) {
         children.remove(child);
      }

   }
}


int changeVolume(const string channel, const int leftVolume, const int rightVolume) {
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
      if(execlp(ALSA_BINARY, ALSA_BINARY, "sset", channel.c_str(), vol.str().c_str(), NULL) == -1) {
         fprintf(stderr, "%s: Failed to execute \"%s\": %s\n", prog, ALSA_BINARY, strerror(errno));
      }
   } else {
      // Wait for the fork to exit and return its exit code
      waitpid(pid, &status, 0);

      if(verbose >= DBL_VERBOSE) {
         printf("%s: %s exited with exit code %d\n", prog, ALSA_BINARY, status);
      }

      return status;
   }

   // How did you get here?
   return FAILURE;
}


int sendMediaKey(unsigned int key) {
   // Connect to X if not done yet
   if(display == NULL) {
      display = XOpenDisplay(NULL);
   }

   if(display == NULL) return FAILURE;

   // Get the keycode
   unsigned int keycode = XKeysymToKeycode(display, key);

   // Simulate the keypress
   if(XTestFakeKeyEvent(display, keycode, 1, 0) == 0) return FAILURE;
   // Release the key
   if(XTestFakeKeyEvent(display, keycode, 0, 0) == 0) return FAILURE;

   // Clear the X buffer which actually sends the key press
   XFlush(display);

   return SUCCESS;
}


void printUsage() {
   printf("%s: Usage: %s [options]\n\
          --port\t(-p) [port]\tSpecify port number\n\
          --encrypt\t(-e)\t\tEncrypt communications\n\
          --no-fork\t(-f)\t\tDon't fork process on startup\n\
          --verbose\t(-v)\t\tIncrease verbosity up to three levels\n\
          --version\t(-V)\t\tPrint version\n\
          --help\t(-h)\t\tDisplay this message\n", prog, prog);
}


void printVersion() {
   printf("%s: %s\n", NAME, VERSION);
}

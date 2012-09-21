// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include "Server.h"
#include "Constants.h"

#define NAME    "Alsa Control Server"
#define VERSION "3.2.0-beta"

#define DEFAULT_PORT 4242

// The program to actually change the volume
#define ALSA_BINARY "amixer"

using namespace std;


char *prog;              // Name of the program
int useEnc;              // Use encryption or not
int clientEnc;           // If the client requested encryption
int verbose;             // The verbose level
Server server;           // The main server object
Client client;           // Client connected to the server
list<int> children;      // List of PIDs of all children created
Display *display;        // Connection to X server


int clientHandshake();

int processCommand();

int parseVolCommand(const string commandArg);

int changeVolume(const string channel, const int leftVolume, const int rightVolume);

int sendMediaKey(unsigned int key);

void sigHandler(const int signal);

void printUsage();

void printVersion();

// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>

#include "Server.h"


#define NAME "Alsa Control Server"
#define VERSION "3.1.0-beta"

#define DEFAULT_PORT 4242
#define NORMAL_EXIT 0
#define ABNORMAL_EXIT 1
#define PARSE_ERROR 2
#define P_BUFFER 512

#define NO_VERBOSE 0
#define VERBOSE 1
#define DBL_VERBOSE 2
#define TPL_VERBOSE 3

using namespace std;


char *prog;       // Name of the program
int verbose;      // The verbose level
Server server;    // The main server object
Client client;    // Clients connected to the server


static void sigHandler(int);

int parseCommand(string command);

FILE* changeVolume(string channel, int leftVolume, int rightVolume);

void printUsage();

void printVersion();

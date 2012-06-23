// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include <vector>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef SERVER_H
#include "Server.h"
#endif

#ifndef CONSTANTS_H
#include "Constants.h"
#endif

#define NAME "Alsa Control Server"
#define VERSION "3.2.0-beta"

#define DEFAULT_PORT 4242
#define NORMAL_EXIT 0
#define ABNORMAL_EXIT 1
#define PARSE_ERROR 2
#define P_BUFFER 512


using namespace std;


char *prog;              // Name of the program
int verbose;             // The verbose level
Server server;           // The main server object
Client client;           // Clients connected to the server
vector<int> children;    // Array of PIDs of all children created


static void sigHandler(const int);

int parseCommand(const string command);

int changeVolume(const string channel, const int leftVolume, const int rightVolume);

void printUsage();

void printVersion();

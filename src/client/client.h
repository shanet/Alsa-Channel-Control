// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "Client.h"
#include "Constants.h"


#define NAME    "Alsa Control Client"
#define VERSION "3.2.0-beta"

#define DEFAULT_CLIENT "127.0.0.1"
#define DEFAULT_PORT   4242

using namespace std;


int    verbose;   // Level of verbosity to use
char   *prog;     // Name of the program
int    useEnc;    // Encrypt communications with the server
Client client;    // The client object that communicates with the server

int serverHandshake();

int sendVolCmd(vector<string> channels, vector<string> vols);

int sendMediaCmd(int commandType);

void printUsage();

void printVersion();

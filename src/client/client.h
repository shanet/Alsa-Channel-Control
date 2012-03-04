//#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "Client.h"


#define NAME "Alsa Control Client"
#define VERSION "3.0.0-beta"

#define DEFAULT_CLIENT "127.0.0.1"
#define DEFAULT_PORT 4242
#define NORMAL_EXIT 0
#define ABNORMAL_EXIT 1

#define NO_VERBOSE 0
#define VERBOSE 1
#define DBL_VERBOSE 2
#define TPL_VERBOSE 3


using namespace std;


int verbose;    // Level of verbosity to use
char *prog;     // Name of the program

void printUsage();

void printVersion();
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include "Client.h"

#define NAME "Alsa Control Client"
#define VERSION "1.0.0-beta"

#define DEFAULT_CLIENT "127.0.0.1"
#define DEFAULT_PORT 4242
#define NORMAL_EXIT 0

#define DEBUG

using namespace std;

void printUsage();

void printVersion();
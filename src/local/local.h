#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include "alsa_vol.h"

// Constants
#define PROG_NAME "Alsa_Control"
#define VERSION 2.0
#define SUCCESS 0
#define FAILURE 1
#define PARSE_ERROR 2

using namespace std;

void showHelp();
void showVersion();

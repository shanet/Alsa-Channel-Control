// Shane Tully
// shanetully.com
// Version 1.1

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <string>
#include <sstream>
#include <iostream>

// Constants
#define VERSION 1.1
#define FRONT 0
#define PCM 1
#define SUCCESS 0
#define FAILURE 1

using namespace std;

void showHelp();
void showVersion();
bool changeVolume(int leftVolume, int rightVolume, int channel=FRONT);


int main(int argc, char* argv[]) {
   int *volumePercent = new int;
   int status = SUCCESS;

   // If wrong number of  args or help requested, show help
   if(argc != 2 || strcmp(argv[1], "--help") == 0) {
      showHelp();
      return FAILURE;
   // Show version and exit if requested
   } else if(strcmp(argv[1], "--version") == 0) {
      showVersion();
      return SUCCESS;
   // Convert "mute" or "unmute" to 0 and 100 respectively
   } else if(strcmp(argv[1], "mute") == 0)
      *volumePercent = 0;
   else if(strcmp(argv[1], "unmute") == 0)
      *volumePercent = 100;
   // Assume anything else is a volume percent. atoi returns 0 on failure so explicitly check for 0 in
   // the args.
   else if((atoi(argv[1]) != 0 || argv[1][0] == '0') && atoi(argv[1]) >= 0 && atoi(argv[1]) <= 100)
      *volumePercent = atoi(argv[1]);
   else {
      showHelp();
      return FAILURE;
   }

   // Execute command for both PCM and Front channels
   if(!changeVolume(100, *volumePercent, FRONT) || !changeVolume(100, *volumePercent, PCM)) {
      cout << "Error: Failed to run Alsa command." << endl;
      status = FAILURE;
   }

   // Clean up pointers
   delete volumePercent;
   volumePercent = NULL;

   return status;
}

bool changeVolume(int leftVolume, int rightVolume, int channel) {
   stringstream ss;
   string command;

   // 100% is the percent of the left channel which should always be 100%
   ss << "amixer sset " << ((channel == 0) ? "'Front'" : "'PCM'") << " " << leftVolume << "%," << rightVolume << "%";
   command = ss.str();

   // Execute command and return if it was successful
   return popen(command.c_str(), "r");
}

void showHelp() {
   cout << "Usage: downstairsVolume [volume percent]\n\twhere volume percent is an intger 0-100 or \"mute\" for 0% or \"unmute\" for 100%." << endl;
}

void showVersion() {
   cout << "Version " << VERSION << endl;
}

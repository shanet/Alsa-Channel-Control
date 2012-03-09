// Shane Tully
// shanetully.com
// Alsa Volume Control
// Version 2.0

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <string>
#include <sstream>
#include <iostream>
#include <vector>


// Constants
#define PROG_NAME "Alsa_Control"
#define VERSION 2.0
#define SUCCESS 0
#define FAILURE 1
#define PARSE_ERROR 2

using namespace std;

void showHelp();
void showVersion();
bool changeVolume(string channel, int leftVolume, int rightVolume);


int main(int argc, char* argv[]) {
   int equalsIndex;
   string rawInput;
   vector<string> channels;
   vector<int> leftVols, rightVols;

   // If wrong number of  args or help requested, show help
   if(argc == 1 || strcmp(argv[1], "--help") == 0) {
      showHelp();
      return PARSE_ERROR;

   // Show version and exit if requested
   } else if(strcmp(argv[1], "--version") == 0) {
      showVersion();
      return SUCCESS;
   }
   
   // Parse args
   for(int i=1; i<argc; i++) {
      rawInput = argv[i];

      // Get the equals index
      if((equalsIndex = rawInput.find('=')) == string::npos) {
         cout << "Invalid arguments: \"" << rawInput << "\"." << endl;
         showHelp();
         return PARSE_ERROR;
      }

      channels.push_back(rawInput.substr(0, equalsIndex));
      leftVols.push_back(atoi((rawInput.substr(rawInput.find('=')+1, rawInput.find(','))).c_str()));
      rightVols.push_back(atoi((rawInput.substr(rawInput.find(',')+1)).c_str()));
   }

   // All vector lengths should be the same
   if(channels.size() != leftVols.size() || channels.size() != rightVols.size()) {
      showHelp();
      return PARSE_ERROR;
   }

   // Convert "mute" or "unmute" to 0 and 100 respectively
   /*} else if(strcmp(argv[1], "mute") == 0)
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
   }*/

   // Execute command for both PCM and Front channels
   for(unsigned int i=0; i<channels.size(); i++) {
      if(!changeVolume(channels[i], leftVols[i], rightVols[i]))
         cout << "Failed to run Alsa command on \"" << channels[i] << '=' << leftVols[i] << ',' << rightVols[i] << "\"." << endl;
   }

   return SUCCESS;
}

bool changeVolume(string channel, int leftVolume, int rightVolume) {
   stringstream command;

   // 100% is the percent of the left channel which should always be 100%
   command << "amixer sset " << channel << " " << leftVolume << "%," << rightVolume << "%";

   // Execute command and return if it was successful
   return popen(command.str().c_str(), "r");
}

void showHelp() {
   cout << "Usage: " << PROG_NAME << " [channel]=[left volume],[right volume]\n\t\tMultiple arguments are allowed. Channels are case sensititve." << endl;
}

void showVersion() {
   cout << PROG_NAME << " version " << VERSION << endl;
}

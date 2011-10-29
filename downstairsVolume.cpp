#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <iostream>

#define BUFFER 100

using std::cout;
using std::endl;

void showHelp();
void executeCommand(char* volumePercent, int channel=0);

int main(int argc, char* argv[]) {
   char *volumePercent = new char[BUFFER/10];

   // If wrong number of  args or help requested, show help
   if(argc != 2 || strcmp(argv[1], "--help") == 0)
      showHelp();
   // Convert "mute" or "unmute" to 0 and 100 respectively
   else if(strcmp(argv[1], "mute") == 0)
      strcpy(volumePercent, "0");
   else if(strcmp(argv[1], "unmute") == 0)
      strcpy(volumePercent, "100");
   // Assume anything else is a volume percent. Convert it to an int and copy it to volumePercent
   else
      sprintf(volumePercent, "%d", atoi(argv[1]));

   // Execute command for both PCM and Front channels
   executeCommand(volumePercent, 0);
   executeCommand(volumePercent, 1);

   // Clean up
   delete volumePercent;
   volumePercent = NULL;

   return 0;
}

void executeCommand(char* volumePercent, int channel) {
   // Channel: 0 = front, 1 = PCM

   char *command = new char[BUFFER];

   // 100% is the percent of the left channel which should always be 100%
   strcpy(command, "amixer sset ");
   strcat(command, ((channel == 0) ? "'Front'" : "'PCM'"));
   strcat(command, " 100%,");

   // Add percent to command (%% is escape for percent sign since Alsa expects a % after the volume percent)
   sprintf(command, "%s%s%%", command, volumePercent);

   // Execute command
   cout << command << endl;
   popen(command, "r");

   // Clean up
   delete command;
   command = NULL;
}

void showHelp() {
   cout << "TODO" << endl;
}

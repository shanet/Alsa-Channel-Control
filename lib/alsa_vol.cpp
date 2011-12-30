// Shane Tully
// shanetully.com

#include "alsa_vol.h"

FILE* changeVolume(string channel, int leftVolume, int rightVolume) {
   stringstream command;

   // 100% is the percent of the left channel which should always be 100%
   command << "amixer sset " << channel << " " << leftVolume << "%," << rightVolume << "%";

   // Execute command and return if it was successful
   return popen(command.str().c_str(), "r");
}

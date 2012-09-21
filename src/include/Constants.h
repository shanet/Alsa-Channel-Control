// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

#ifndef CONSTANTS_H
#define CONSTANTS_H

// Program-wide constants
#define SUCCESS 0
#define FAILURE -1
#define END     2
#define BUFFER  1024

#define NORMAL_EXIT   0
#define ABNORMAL_EXIT 1
#define PARSE_ERROR   2

#define NO_VERBOSE  0
#define VERBOSE     1
#define DBL_VERBOSE 2
#define TPL_VERBOSE 3

#define CMD_VOL  0
#define CMD_PLAY 1
#define CMD_NEXT 2
#define CMD_PREV 3

#define ENC_NONE 0
#define ENC_RSA  1
#define ENC_AES  2

#define XF86AudioPlay 0x1008ff14
#define XF86AudioNext 0x1008ff17
#define XF86AudioPrev 0x1008ff16
#define XF86AudioStop 0x1008ff15

#endif
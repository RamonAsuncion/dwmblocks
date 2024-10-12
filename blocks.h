#ifndef _BLOCKS_H_
#define _BLOCKS_H_

typedef struct {
  char* icon;
  char* command;
  unsigned int interval;
  unsigned int signal;
} StatusBlock;

static const StatusBlock blocks[] = {
  /*icon */  /*command*/ /* update interval*/ /* update signal*/
  {"", "date '+%Y-%m-%d %H:%M'", 5, 0},
  //{"", "./scripts/network.sh", 5, 0},
};

#endif // _BLOCKS_H_


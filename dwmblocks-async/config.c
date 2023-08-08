#include "config.h"

#include "block.h"
#include "util.h"

Block blocks[] = {
//   Command, Update Interval, Update Signal
    {"free -h | awk '/^Mem/ { print $3\"/\"$2 }' | sed s/i//g",	30,	0},
};

const unsigned short blockCount = LEN(blocks);

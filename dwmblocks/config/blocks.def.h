//Modify this file to change what commands output to your statusbar, and recompile using the make command.
static const Block blocks[] = {
//  Icon                Command                                                         Update Interval         Update Signal
    { "No blocks on, check dwmblocks/config/blocks.h | ", "",                             0,                      0 },   // Remove after customization
    //{ " Ram: ",         "free -h | awk '/^Mem/ { print $3\"/\"$2 }' | sed s/i//g",      30,                     0 },   // Ram / Memory (slightly bugged in deb based systems, issue with original dwmblocks I think)
    //{ " ",              "date '+%B %d (%a) %H:%M'",                                     5,                      0 },   // Date (Month Day (Day Name) 24h-time)
    //{ " CPU Usage: ",   "echo ''$[100-$(vmstat 1 2|tail -1|awk '{print $15}')]%",       5,                      0 },   // Cpu usage
    //{ " ",              "",                                                             0,                      0 },   // Trailing spacer
};

//sets delimeter between status commands. NULL character ('\0') means no delimeter.
static char delim[] = " |";
static unsigned int delimLen = 5;

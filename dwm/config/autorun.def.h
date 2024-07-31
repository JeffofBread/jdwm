//////////////////////////////////////////////////////////////////////////////////////////////
//                               _                   _                    _                 //
//               /\             | |                 | |                  | |                //
//              /  \     _   _  | |_    ___    ___  | |_    __ _   _ __  | |_               //
//             / /\ \   | | | | | __|  / _ \  / __| | __|  / _` | | '__| | __|              //
//            / ____ \  | |_| | | |_  | (_) | \__ \ | |_  | (_| | | |    | |_               //
//           /_/    \_\  \__,_|  \__|  \___/  |___/  \__|  \__,_| |_|     \__|              //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

// For autostarting programs and scripts at jeff_dwm startup. 
// Make sure each argument is its own string, with commas between, 
// and a NULL at the end followed by a comma. The end of the array
// should always end 'NULL,'.
static const char *const autostart[] = {
    "jeff_dwm_java.sh", NULL,           // Exports java vars to help some java apps work better with jeff_dwm
    "jeff_dwm_autostarts.sh", NULL,     // Runs jeff_dwm_autostart.sh to start any programs you want at login
    "dwmblocks", NULL,                  // dwmblocks, replaces the status text with info specified using scripts. Check out `/jeff_dwm/dwmblocks/config/blocks.h`
    NULL,                               // Defines the end of the array, do not remove. 
};
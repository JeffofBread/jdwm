//////////////////////////////////////////////////////////////////////////////////////////////
//                               _                   _                    _                 //
//               /\             | |                 | |                  | |                //
//              /  \     _   _  | |_    ___    ___  | |_    __ _   _ __  | |_               //
//             / /\ \   | | | | | __|  / _ \  / __| | __|  / _` | | '__| | __|              //
//            / ____ \  | |_| | | |_  | (_) | \__ \ | |_  | (_| | | |    | |_               //
//           /_/    \_\  \__,_|  \__|  \___/  |___/  \__|  \__,_| |_|     \__|              //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

// For autostarting programs and scripts at dwm startup.
static const char *const autostart[] = {
    "jeff_dwm_java.sh", NULL,           // Exports java vars to help them java apps play nice with dwm
    "jeff_dwm_autostarts.sh", NULL,     // Uses jeff_dwm_autostart.sh to start any programs you want at login
    "dwmblocks", NULL,                  // Replaces the status text with info specified in blocks, check /jeff_dwm/dwmblocks/
    NULL,
};
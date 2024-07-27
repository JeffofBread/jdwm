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
    "jeff_dwm_java.sh", NULL, // Exports java vars to help them java apps play nice with dwm

    // My xrandr config, just spread across multiple lines
    /* "xrandr", "--output", "DP-0", "--off", "--output", "DP-1", "--off", "--output DP-2", "--off" "--output", "DP-3", "--off",
    "--output", "HDMI-0", "--off", "--output", "HDMI-1", "--mode", "1920x1080", "--rate", "75.00", "--pos", "0x180", "--rotate",
    "normal", "--output", "HDMI-2", "--off", "--output", "DP-4", "--primary", "--mode", "2560x1440", "--rate", "240.00",
    "--pos", "1920x0", "--rotate", "normal", "--output", "DP-5", "--off", NULL, */

    // Below are examples, not necessary:
    //---------------------------------//
	//"picom", "--animations", "-b", NULL,  // Compositor, I recommend https://github.com/FT-Labs/picom
	//"dunst", NULL,	// Deals with app notifications
	//"dwmblocks", NULL,  // Sets the status on the bar with time, memory usage, etc
	//"playerctld", "daemon", NULL,  // Background daemon to improve playerctld commands like prev, next, etc
	//"nm-applet", NULL,  // Simple network tray app
	//"flameshot", NULL,  // Screenshot tray tool
	//"pasystray", NULL,	// Simple pulse audio tray app
	//"jetbrains-toolbox", "--minimize", NULL,  // Jetbrains toolbox app for jetbrains software
	NULL,
};
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
	"jeff_dwm_setup.sh", NULL,

    // Below are examples, not necessary:
    //---------------------------------//
	//"picom", NULL,  // Compositor, I recommend https://github.com/FT-Labs/picom
	//"dunst", NULL,	// Deals with app notifications
	//"dwmblocks", NULL,  // Sets the status on the bar with time, memory usage, etc
	//"playerctld", "daemon", NULL,  // Background daemon to improve playerctld commands like prev, next, etc
	//"nm-applet", NULL,  // Simple network tray app
	//"flameshot", NULL,  // Screenshot tray tool
	//"pasystray", NULL,	// Simple pulse audio tray app
	//"jetbrains-toolbox", "--minimize", NULL,  // Jetbrains toolbox app for jetbrains software
	NULL,
};
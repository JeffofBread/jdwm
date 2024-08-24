//////////////////////////////////////////////////////////////////////////////////////////////
//                               _                   _                    _                 //
//               /\             | |                 | |                  | |                //
//              /  \     _   _  | |_    ___    ___  | |_    __ _   _ __  | |_               //
//             / /\ \   | | | | | __|  / _ \  / __| | __|  / _` | | '__| | __|              //
//            / ____ \  | |_| | | |_  | (_) | \__ \ | |_  | (_| | | |    | |_               //
//           /_/    \_\  \__,_|  \__|  \___/  |___/  \__|  \__,_| |_|     \__|              //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

// For autostarting programs and scripts at jdwm startup. 
// Make sure each argument is its own string, with commas between, 
// and a NULL at the end followed by a comma. The end of the array
// should always end 'NULL,'.
static const char *const autostart[] = {
    NULL,                               // Defines the end of the array, do not remove. 
};
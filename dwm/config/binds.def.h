//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                 _  __                 _       _               _                          //
//                | |/ /                | |     (_)             | |                         //
//                | ' /    ___   _   _  | |__    _   _ __     __| |  ___                    //
//                |  <    / _ \ | | | | | '_ \  | | | '_ \   / _` | / __|                   //
//                | . \  |  __/ | |_| | | |_) | | | | | | | | (_| | \__ \                   //
//                |_|\_\  \___|  \__, | |_.__/  |_| |_| |_|  \__,_| |___/                   //
//                                __/ |                                                     //
//                               |___/                                                      //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


static const Key keys[] = {


//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Tag Related Keys: */

        #define TAGKEYS(KEY,TAG) \
        { Win,           KEY,      view,         {.ui = 1 << TAG} }, \
        { Win|Control,   KEY,      toggleview,   {.ui = 1 << TAG} }, \
        { Win|Shift,     KEY,      tag,          {.ui = 1 << TAG} }, \
        { Win|Shift,     KEY,      view,         {.ui = 1 << TAG} }, \
        { Win|Alt,       KEY,      viewall,      {.ui = 1 << TAG} }, \


        //Modifier       Key        Function         Argument
        TAGKEYS(         XK_1,                       0)
        TAGKEYS(         XK_2,                       1)
        TAGKEYS(         XK_3,                       2)
        TAGKEYS(         XK_4,                       3)
        TAGKEYS(         XK_5,                       4)
        TAGKEYS(         XK_6,                       5)
        TAGKEYS(         XK_7,                       6)
        TAGKEYS(         XK_8,                       7)
        TAGKEYS(         XK_9,                       8)
        { Win,           XK_Tab,    view,            {0} },
        { Win,           XK_0,      view,            {.ui = ~0 } },
        { Win|Shift,     XK_0,      tag,             {.ui = ~0 } },
        { Win|Shift,     XK_Up,     shiftboth,       { .i = +1 } },
        { Win|Shift,     XK_Down,   shiftboth,       { .i = -1 } },
        //{ Win|Control,   XK_o,      shiftviewclients,{ .i = +1 } }, Bugged, will fix or remove
        //{ Win|Control,   XK_i,      shiftviewclients,{ .i = -1 } }, Bugged, will fix or remove
        { Win,           XK_o,      shiftview,       { .i = +1 } },
        { Win,           XK_i,      shiftview,       { .i = -1 } },
        

        // Used to send the mouse as well as the window to a new monitor.
        // Is it simple? Yes. Is it lazy? Yes. Is it the best option? Maybe, but im lazy.
        { Win|Shift,     XK_Left,     tagmon,      {.i = -1 } },
        { Win|Shift,     XK_Right,    tagmon,      {.i = +1 } },
        { Win|Shift,     XK_Left,     focusmon,    {.i = -1 } },
        { Win|Shift,     XK_Right,    focusmon,    {.i = +1 } },


//////////////////////////////////////////////////////////////////////////////////////////////

                              /* Window Geometry Related Keys: */

        //Modifier         Key       Function                Argument
        { Win,             XK_f,     togglefullscreen,       {0} },
        { Win|Shift,       XK_f,     togglefakefullscreen,   {0} },


        // Vanity Gap Keybinds, kind of a mess by default. 
        // If you plan to use them, I recommend updating them to your choice keybinds
        { Win|Alt,          XK_u,   incrgaps,       {.i = +1 } },
        { Win|Alt|Shift,    XK_u,   incrgaps,       {.i = -1 } },
        { Win|Alt,          XK_i,   incrigaps,      {.i = +1 } },
        { Win|Alt|Shift,    XK_i,   incrigaps,      {.i = -1 } },
        { Win|Alt,          XK_o,   incrogaps,      {.i = +1 } },
        { Win|Alt|Shift,    XK_o,   incrogaps,      {.i = -1 } },
        { Win|Alt,          XK_6,   incrihgaps,     {.i = +1 } },
        { Win|Alt|Shift,    XK_6,   incrihgaps,     {.i = -1 } },
        { Win|Alt,          XK_7,   incrivgaps,     {.i = +1 } },
        { Win|Alt|Shift,    XK_7,   incrivgaps,     {.i = -1 } },
        { Win|Alt,          XK_8,   incrohgaps,     {.i = +1 } },
        { Win|Alt|Shift,    XK_8,   incrohgaps,     {.i = -1 } },
        { Win|Alt,          XK_9,   incrovgaps,     {.i = +1 } },
        { Win|Alt|Shift,    XK_9,   incrovgaps,     {.i = -1 } },
        { Win|Alt,          XK_0,   togglegaps,     {0} },
        { Win|Alt|Shift,    XK_0,   defaultgaps,    {0} },

//////////////////////////////////////////////////////////////////////////////////////////////

                                  /* Layout Related Keys: */

        //Modifier          Key             Function           Argument
        { Win,              XK_t,           setlayout,         {.v = &layouts[0]} },
        { Win,              XK_e,           setlayout,         {.v = &layouts[1]} },
        { Win,              XK_m,           setlayout,         {.v = &layouts[2]} },
        { Win,              XK_l,           spawn,             SHCMD( "rofi_layoutmenu.sh " ROFITHEME ) },
        { Win,              XK_Up,          focusstack,        {.i = +1 } },
        { Win,              XK_Down,        focusstack,        {.i = -1 } },
        { Win|Control,      XK_i,           incnmaster,        {.i = +1 } },
        { Win|Control,      XK_d,           incnmaster,        {.i = -1 } },
        { Win|Shift,        XK_h,           setmfact,          {.f = -0.05} },
        { Win|Shift,        XK_l,           setmfact,          {.f = +0.05} },
        { Win|Shift,        XK_Return,      zoom,              {0} },
        { Win,              XK_comma,       layoutscroll,      {.i = -1 } },
        { Win,              XK_period,      layoutscroll,      {.i = +1 } },
        { Win|Control,      XK_j,           rotatestack,       {.i = +1 } },
        { Win|Control,      XK_k,           rotatestack,       {.i = -1 } },
        { Win|Shift,        XK_space,       togglefloating,    {0} },
        { Win|Control,      XK_space,       centerfloating,    {0} },

//////////////////////////////////////////////////////////////////////////////////////////////

                                /* jeff_dwm misc Keys: */

        //Modifier              Key             Function          Argument
        { Alt,                  XK_Tab,         spawn,            SHCMD( "rofi -no-fixed-num-lines -show window -theme " ROFITHEME ) }, // Window Switcher    
        { Win,                  XK_d,           spawn,            SHCMD( "rofi -no-fixed-num-lines -show drun -theme " ROFITHEME ) }, // App Launcher
        { Win,                  XK_p,           spawn,            SHCMD( "rofi_powermenu.sh " ROFITHEME ) }, // Power Menu 
        { Win,                  XK_k,           spawn,            SHCMD( "rofi -show calc -modi calc -no-show-match -no-sort -theme " ROFITHEME ) }, // Calculator
        { Win,                  XK_h,           spawn,            SHCMD( "rofi_jeff_dwm_theme.sh " ROFITHEME ) },
        { Win,                  XK_w,           spawn,            SHCMD( "BROWSER" ) },
        { Win,                  XK_c,           spawn,            SHCMD( "CODE_EDITOR" ) },
        { Win,                  XK_a,           spawn,            SHCMD( "FILE_MANAGER" ) },
        { Win,                  XK_Return,      spawn,            SHCMD( "TERM" ) },
        { Win,                  XK_grave,       togglescratch,    SHCMD( "SCRATCHPAD" ) },
        { Win|Shift,            XK_grave,       spawn,            SHCMD( "SCRATCHPAD" ) },
        { Control|Shift,        XK_q,           spawn,            SHCMD( "RECOMPILE_TERM" ) },
        { Win,                  XK_Left,        focusmon,         {.i = -1 } },
        { Win,                  XK_Right,       focusmon,         {.i = +1 } },
        { Win,                  XK_s,           togglesticky,     {0} },
        { Win,                  XK_q,           killclient,       {0} },
        { Win|Shift,            XK_b,           togglebar,        {0} },
        { Win|Shift,            XK_q,           quit,             {1} },
        { Win|Control|Shift,    XK_q,           quit,             {0} },

//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Media keys: */

        //Modifier              Key             Function          Argument
        { Win,                  PageUp,         spawn,            SHCMD( "pamixer -i 5" ) },
        { Win,                  PageDown,       spawn,            SHCMD( "pamixer -d 5" ) },
        { Win,                  XK_Scroll_Lock, spawn,            SHCMD( "playerctl next" ) },
        { Win,                  XK_Print,       spawn,            SHCMD( "playerctl previous" ) },
        { Win,                  XK_Home,        spawn,            SHCMD( "playerctl position 10+" ) },
        { Win,                  XK_End,         spawn,            SHCMD( "playerctl position 10-" ) },
        { Win,                  XK_Pause,       spawn,            SHCMD( "playerctl play-pause" ) },
        { Win,                  XK_Insert,      spawn,            SHCMD( "pactl set-sink-mute @DEFAULT_SINK@ toggle" ) },
        { Win,                  XK_Delete,      spawn,            SHCMD( "pactl set-source-mute @DEFAULT_SOURCE@ toggle" ) },

//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Special Keys: */

        // Note: I have only done some as examples, there are MANY more that can be found in
        // keydefs.h / keydefs.def.h. There are hundreds of unbound keys that exist there,
        // if your keyboard has them. 

        //Modifier      Key                         Function    Argument
        { 0,            XK_AudioRaiseVolume,        spawn,      SHCMD("pamixer -i 5" ) },                  
        { 0,            XK_AudioLowerVolume,        spawn,      SHCMD("pamixer -d 5" ) },
        { 0,            XK_AudioNext,               spawn,      SHCMD("playerctl next" ) },
        { 0,            XK_AudioPrev,               spawn,      SHCMD("playerctl previous") },
        { 0,            XK_AudioPlay,               spawn,      SHCMD("playerctl play") },
        { 0,            XK_AudioStop,               spawn,      SHCMD("playerctl pause") },
        { 0,            XK_AudioRewind,             spawn,      SHCMD("playerctl position 10-") },
        { 0,            XK_AudioForward,            spawn,      SHCMD("playerctl position 10+") },
        { 0,            XK_AudioMute,               spawn,      SHCMD("pactl set-sink-mute @DEFAULT_SINK@ toggle") },
        { 0,            XK_AudioMicMute,            spawn,      SHCMD("pactl set-source-mute @DEFAULT_SOURCE@ toggle") },
        { 0,            XK_PowerOff,                spawn,      SHCMD("rofi_powermenu.sh " ROFITHEME ) },
        { 0,            XK_Sleep,                   spawn,      SHCMD("systemctl suspend") }, 
        { 0,            XK_Standby,                 spawn,      SHCMD("systemctl suspend") },
        { 0,            XK_Suspend,                 spawn,      SHCMD("systemctl suspend") },
        { 0,            XK_Hibernate,               spawn,      SHCMD("systemctl hibernate") },
        { 0,            XK_Calculator,              spawn,      SHCMD( "rofi -show calc -modi calc -no-show-match -no-sort -theme " ROFITHEME ) },

        // Below are not set up, as it varies a lot between systems. Use a program that works for your specific system.
        //{ 0,          XK_MonBrightnessUp,         spawn,      SHCMD("") },
        //{ 0,          XK_MonBrightnessDown,       spawn,      SHCMD("") },
        //{ 0,          XK_KbdBrightnessUp,         spawn,      SHCMD("") },
        //{ 0,          XK_KbdBrightnessDown,       spawn,      SHCMD("") },
        //{ 0,          XK_KbdLightOnOff,           spawn,      SHCMD("") },
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//       ____            _     _                       ____    _               _            //
//      |  _ \          | |   | |                     |  _ \  (_)             | |           //
//      | |_) |  _   _  | |_  | |_    ___    _ __     | |_) |  _   _ __     __| |  ___      //
//      |  _ <  | | | | | __| | __|  / _ \  | '_ \    |  _ <  | | | '_ \   / _` | / __|     //
//      | |_) | | |_| | | |_  | |_  | (_) | | | | |   | |_) | | | | | | | | (_| | \__ \     //
//      |____/   \__,_|  \__|  \__|  \___/  |_| |_|   |____/  |_| |_| |_|  \__,_| |___/     //
//                                                                                          //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


// click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin
static const Button buttons[] = {


//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Left Mouse Button: */

        //Click           Key            Button          Function        Argument 
        { ClkLtSymbol,    0,             LeftClick,      setlayout,      {0} },
        { ClkTagBar,      0,             LeftClick,      view,           {0} },

        // Used to send focus and the window to a new monitor.
        // Is it simple? Yes. Is it lazy? Yes. Is it the best option? Maybe, but im lazy.
        { ClkTagBar,      Win,          LeftClick,      tag,            {0} },
        { ClkTagBar,      Win,          LeftClick,      view,           {0} },

        { ClkClientWin,   Win,          LeftClick,      movemouse,      {0} },
        { ClkClientWin,   Win|Control,  LeftClick,      floatandmove,   {0} },


//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Middle Mouse Button: */

        //Click           Key           Button           Function          Argument
        { ClkWinTitle,    0,            MiddleClick,     spawn,            SHCMD( "TERM" ) }, 
        { ClkLtSymbol,    0,            MiddleClick,     spawn,            SHCMD( "rofi_jeff_dwm_theme.sh " ROFITHEME ) },
        { ClkClientWin,   Win,          MiddleClick,     togglefloating,   {0} },


//////////////////////////////////////////////////////////////////////////////////////////////

                               /* Right Mouse Button: */

        //Click           Key           Button             Function        Argument
        { ClkLtSymbol,    0,            RightClick,        spawn,          SHCMD( "rofi_layoutmenu.sh " ROFITHEME ) },
        { ClkTagBar,      0,            RightClick,        toggleview,     {0} },
        { ClkClientWin,   Win,          RightClick,        resizemouse,    {0} },


        // Optional, enables right clicking on the desktop to spawn a jgmenu instance
        //{ ClkRootWin,     0,            RightClick,        spawn,          {.v = jgmenucmd } },

};
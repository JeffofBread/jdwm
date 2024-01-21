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
        { Win|Alt,       KEY,      viewall,      {.ui = 1 << TAG} }, \
        { Win|Shift,     KEY,      view,         {.ui = 1 << TAG} }, \
        { Win|Shift,     KEY,      tag,          {.ui = 1 << TAG} }, \
        { Win|Control,   KEY,      toggleview,   {.ui = 1 << TAG} }, \

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
        { Win|Shift,     XK_o,      shiftboth,       { .i = +1 } },
        { Win|Shift,     XK_i,      shiftboth,       { .i = -1 } },
        { Win,           XK_o,      shiftview,       { .i = +1 } },
        { Win,           XK_i,      shiftview,       { .i = -1 } },
        { Win,           XK_n,      togglealttag,    {0} },
        

        // Used to send the mouse as well as the window to a new monitor.
        // Is it simple? Yes. Is it lazy? Yes. Is it the best option? Maybe, but im lazy.
        { Win|Shift,     XK_Left,     tagmon,      {.i = -1 } },
        { Win|Shift,     XK_Right,    tagmon,      {.i = +1 } },
        { Win|Shift,     XK_Left,     focusmon,    {.i = -1 } },
        { Win|Shift,     XK_Right,    focusmon,    {.i = +1 } },


        // With my implimentation these are redundant, but kept here in case you wish to re-enable them.
        // As it stands, I send the view with any window sent to another tag, meaning that these and
        // shiftview/shiftboth do the exact same thing as the below functions. 
        /*{ Win|Shift,     XK_h,      shiftviewclients,    { .i = -1 }},
        { Win|Shift,     XK_l,      shiftviewclients,    { .i = +1 }},
        { Win|Control,   XK_h,      shiftswaptags,       { .i = -1 }},
        { Win|Controlm   XK_l,      shiftswaptags,       { .i = +1 }},*/


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
        { Win,              XK_Up,          focusstack,        {.i = +1 } },
        { Win,              XK_Down,        focusstack,        {.i = -1 } },
        { Win|Control,      XK_i,           incnmaster,        {.i = +1 } },
        { Win|Control,      XK_d,           incnmaster,        {.i = -1 } },
        { Win,              XK_h,           setmfact,          {.f = -0.05} },
        { Win,              XK_l,           setmfact,          {.f = +0.05} },
        { Win|Shift,        XK_Return,      zoom,              {0} },
        { Win,              XK_comma,       layoutscroll,      {.i = -1 } },
        { Win,              XK_period,      layoutscroll,      {.i = +1 } },
        { Win|Control,      XK_j,           rotatestack,       {.i = +1 } },
        { Win|Control,      XK_k,           rotatestack,       {.i = -1 } },
        { Win|Shift,        XK_space,       togglefloating,    {0} },
        { Win|Control,      XK_space,       centerfloating,    {0} },

//////////////////////////////////////////////////////////////////////////////////////////////

                                                                 /* jeff_dwm Keys: */

        //Modifier              Key             Function          Argument
        { Win|Shift,            XK_b,           togglebar,        {0} },
        { Win,                  XK_q,           killclient,       {0} },
        { Win,                  XK_Left,        focusmon,         {.i = -1 } },
        { Win,                  XK_Right,       focusmon,         {.i = +1 } },
        { Win,                  XK_d,           spawn,            {.v = rofi_launcher_cmd } },
        { Win,                  XK_Return,      spawn,            {.v = termcmd } },
        { Control|Shift,        XK_q,           spawn,            {.v = recompilecmd } },
        { Alt,                  XK_Tab,         spawn,            {.v = alttabcmd } },
        { Win|Shift,            XK_grave,       spawn,            {.v = scratchpadcmd } },
        { Win,                  XK_grave,       togglescratch,    {.v = scratchpadcmd } },
        { Win|Shift,            XK_u,           focusurgent,      {0} },
        { Win,                  XK_s,           togglesticky,     {0} },
        { Win,                  XK_u,           swalstopsel,      {0} },
        { Win|Shift,            XK_q,           quit,             {1} },
        { Win|Control|Shift,    XK_q,           quit,             {0} },

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
        { ClkClientWin,   Win|Shift,     LeftClick,      swalmouse,      {0} },
        { ClkLtSymbol,    0,             LeftClick,      setlayout,      {0} },
        { ClkTagBar,      0,             LeftClick,      view,           {0} },


        // Used to send focus and the window to a new monitor.
        // Is it simple? Yes. Is it lazy? Yes. Is it the best option? Maybe, but im lazy.
        { ClkTagBar,    Win,     LeftClick,     tag,     {0} },
        { ClkTagBar,    Win,     LeftClick,     view,    {0} },


        /* placemouse options, choose which feels more natural:
         *    0 - tiled position is relative to mouse cursor
         *    1 - tiled postiion is relative to window center
         *    2 - mouse pointer warps to window center
         *
         * The moveorplace uses movemouse or placemouse depending on the floating state
         * of the selected client. Set up individual keybindings for the two if you want
         * to control these separately (i.e. to retain the feature to move a tiled window
         * into a floating position).
         */
        { ClkClientWin,   Win,   LeftClick,   moveorplace,   {.i = 0} },


//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Middle Mouse Button: */

        //Click           Key           Button           Function          Argument
        { ClkWinTitle,    0,            MiddleClick,     zoom,             {0} },
        { ClkStatusText,  0,            MiddleClick,     spawn,            {.v = termcmd } },
        { ClkClientWin,   Win,          MiddleClick,     togglefloating,   {0} },


//////////////////////////////////////////////////////////////////////////////////////////////

                               /* Right Mouse Button: */

        //Click           Key           Button             Function        Argument
        { ClkLtSymbol,    0,            RightClick,        spawn,          {.v = layoutmenucmd} },
        { ClkClientWin,   Win,          RightClick,        resizemouse,    {0} },
        { ClkTagBar,      Win,          RightClick,        toggletag,      {0} },
        { ClkTagBar,      0,            RightClick,        toggleview,     {0} },
        { ClkTagBar,      Win,          RightClick,        toggletag,      {0} },


        // Optional, enables right clicking on the desktop to spawn a jgmenu instance
        //{ ClkRootWin,     0,            RightClick,        spawn,          {.v = jgmenucmd } },

};
//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//    __  __   _                       _   _                                                //
//   |  \/  | (_)                     | | | |                                               //
//   | \  / |  _   ___    ___    ___  | | | |   __ _   _ __     ___    ___    _   _   ___   //
//   | |\/| | | | / __|  / __|  / _ \ | | | |  / _` | | '_ \   / _ \  / _ \  | | | | / __|  //
//   | |  | | | | \__ \ | (__  |  __/ | | | | | (_| | | | | | |  __/ | (_) | | |_| | \__ \  //
//   |_|  |_| |_| |___/  \___|  \___| |_| |_|  \__,_| |_| |_|  \___|  \___/   \__,_| |___/  //
//                                                                                          //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

// Window Icons
#define ICONSIZE                                  20            /* window icon size */
#define ICONSPACING                               5             /* space between icon and title */


// Themes:
// ------------------------------------------------------------------------------------------------------------ //

// Stormy blue and purple theme created by @JeffOfBread
static const char darkblue[]    = "#16162F";
static const char midblue[]     = "#418EB4";
static const char lightblue[]   = "#A8CAD3"; 
static const char nearblack[]   = "#0B0D1E";
static const char lightpurple[] = "#645A6D";
static const char orange[]      = "#fa9061";
static const char white[]       = "#ffffff";

static const char *colors[][4] = {

    // Clients       foreground     background      border          float
    [SchemeNorm] = { nearblack,     nearblack,      darkblue,       darkblue  },    // Generic scheme for unfocused clients
    [SchemeSel]  = { nearblack,     nearblack,      lightblue,      lightblue },    // Generic scheme for focused clients

    // Bar                  text        background
    [SchemeSystray]     = { lightblue,  darkblue },   // Systray
    [SchemeStatus]      = { lightblue,  darkblue },   // Statusbar (dwmblocks)
    [SchemeTagsSel]     = { orange,     darkblue },   // Tagbar left selected
    [SchemeTagsNorm]    = { lightblue,  darkblue },   // Tagbar left unselected
    [SchemeInfoSel]     = { lightblue,  darkblue },   // infobar middle selected
    [SchemeInfoNorm]    = { lightblue,  darkblue },   // infobar middle unselected 
};

/* Lush green theme created by @JeffOfBread
static const char greygreen[]   = "#2f4a34";
static const char normalgreen[] = "#253a29";
static const char darkgreen[]   = "#111b13";
static const char bluegreen[]   = "#1e453e";
static const char grey[]        = "#25272a";
static const char white[]       = "#e1e0db";

static const char *colors[][4] = {
    // Clients       foreground     background    border      float
    [SchemeNorm] = { darkgreen,     grey,         darkgreen,  darkgreen },    // Generic scheme for unfocused clients
    [SchemeSel]  = { greygreen,     grey,         greygreen,  greygreen },    // Generic scheme for focused clients

    // Bar                  text        background
    [SchemeSystray]     = { white,      normalgreen },   // Systray
    [SchemeStatus]      = { white,      normalgreen },   // Statusbar (dwmblocks)
    [SchemeTagsSel]     = { white,      normalgreen },   // Tagbar left selected
    [SchemeTagsNorm]    = { white,      normalgreen },   // Tagbar left unselected
    [SchemeInfoSel]     = { white,      normalgreen },   // infobar middle selected
    [SchemeInfoNorm]    = { white,      normalgreen },   // infobar middle unselected
};
*/

/* Default-ish dwm colorscheme created by the suckless team, adapted to by @JeffOfBread
static const char col_gray1[] = "#222222";
static const char col_gray2[] = "#444444";
static const char col_gray3[] = "#bbbbbb";
static const char col_gray4[] = "#eeeeee";
static const char col_cyan[]  = "#005577";

static const char *colors[][4] = {

	// Clients       foreground     background      border          float
	[SchemeNorm] = { col_gray3,     col_gray1,      col_gray2,      col_gray2 },  // Generic scheme for unfocused clients
	[SchemeSel]  = { col_gray4,     col_cyan,       col_gray2,      col_cyan  },  // Generic scheme for focused clients

	// Bar                  text       background
    [SchemeSystray]     = { col_gray3, col_gray1 }, // Systray
	[SchemeStatus]  	= { col_gray3, col_gray1 }, // Statusbar (dwmblocks)
	[SchemeTagsSel]  	= { col_gray4, col_cyan  }, // Tagbar left selected
	[SchemeTagsNorm]  	= { col_gray3, col_gray1 }, // Tagbar left unselected
	[SchemeInfoSel]  	= { col_gray4, col_cyan  }, // infobar middle selected
	[SchemeInfoNorm]  	= { col_gray3, col_gray1 }, // infobar middle unselected
};
*/

// Border
static const unsigned int borderpx              = 0;            /* border pixel of windows */
static const unsigned int fborderpx             = 5;            /* border pixel of floating windows */

// Bar/Status padding
static const int vertpad                        = 10;           /* vertical padding of bar */
static const int vertpadbar                     = 2;            /* vertical padding for statusbar */
static const int sidepad                        = 10;           /* horizontal padding of bar */
static const int horizpadbar                    = 0;            /* horizontal padding for statusbar */

// Tag underline
static const int ulineall                       = 0;            /* 1 to show underline on all tags, 0 for just the active ones */
static const unsigned int ulinepad              = 5;            /* horizontal padding between the underline and tag */
static const unsigned int ulinestroke           = 2;            /* thickness / height of the underline */
static const unsigned int ulinevoffset          = 1;            /* how far above the bottom of the bar the line should appear */

// Bar feature toggles
static const int showsystray                    = 1;            /* 0 means no systray */
static const int showbar                        = 1;            /* 0 means no bar */
static const int topbar                         = 1;            /* 0 means bottom bar */
static const int showtitle                      = 1;            /* 0 means no title */
static const int showtags                       = 1;            /* 0 means no tags */
static const int showlayout                     = 1;            /* 0 means no layout indicator */
static const int showstatus                     = 1;            /* 0 means no status bar */
static const int showfloating                   = 1;            /* 0 means no floating indicator */

// Systray
static const unsigned int systraypinning        = 0;            /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft         = 0;            /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing        = 3;            /* systray spacing */
static const int systraypinningfailfirst        = 1;            /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/

// Misc
static const unsigned int raisefloatwinfoc      = 1;            /* whether to raise floating window on focus, mainly to help deal with overlapping windows */
static const unsigned int centeredwindowname    = 1;            /* 0 is default dwm behavior, 1 centers the name on the monitor width (not the bars), and over 1 is a lazy toggle off for the window name */
static const unsigned int snap                  = 32;           /* snap pixel, don't see a reason you would need to change this */

// Bar Font
static const char *fonts[]                      = { "JetBrainsMono:size=14" };

// Sticky Icon
static const XPoint stickyicon[]        = { {0,0}, {4,0}, {4,8}, {2,6}, {0,8}, {0,0} }; /* represents the icon as an array of vertices */
static const XPoint stickyiconbb        = {4,8}; /* defines the bottom right corner of the polygon's bounding box (speeds up scaling) */

// For autostarting programs and scripts at jdwm startup. 
// Make sure each argument is its own string, with commas between, 
// and a NULL at the end followed by a comma. The end of the array
// should always end 'NULL,'.
static const char *const autostart[] = {
    NULL,                               // Defines the end of the array, do not remove. 
};

// Tag Characters
static const char *tags[]               = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

// Please make sure this matches the name of your terminal defined as SCRATCHPAD in jdwm.aliases.
// Example, if you use Kitty or Alacritty, to spawn a scratchpad use the `-T` flag followed by `Scratchpad`.
// AKA `kitty -T Scratchpad` or `alacritty -T Scratchpad`
static const char  scratchpadname[]     =   "Scratchpad";

// Window Rules
static const Rule rules[] = {

// Using `xprop` you can find the instance, class, and title of any client:
//   WM_CLASS(STRING) = instance, class
//   WM_NAME(STRING) = title
//
// Tag mask (Unsigned Int): What tag a window will reside in when spawned or updated
//
// isfloating (Bool): Pretty self explanitory, whether the window should be floating by default/when spawned
//
// monitor (Int): Determines what monitor a window will reside on when spawned or updated
//  -1 - No monitor specified
//   0 - Monitor 0
//   1 - Monitor 1
//       ..etc
//
// ignoretransient (Bool): Again, self explanitory, for ignoring any window that should be transient, sometimes usefull for Unreal Engine or Jetbrains tools
//
// unmanaged (Int): Determines if jdwm should manage the window or not
//   0 - The window is managed by the window manager
//   1 - The window will be placed above all other windows
//   2 - The window will be placed below all other windows
//   3 - The window is left as-is (neither lowered nor raised)


        // The below are my personal examples, feel free to remove them: 
        /* class                instance    title           tags mask       isfloating       monitor        ignoretransient     unmanaged */
        { NULL,                 NULL,       scratchpadname, 0,              1,              -1,             0,                  0 }, // Used for scratchpad, don't remove
};

//////////////////////////////////////////////////////////////////////////////////////////////
//                  _                                        _                              //
//                 | |                                      | |                             //
//                 | |        __ _   _   _    ___    _   _  | |_   ___                      //
//                 | |       / _` | | | | |  / _ \  | | | | | __| / __|                     //
//                 | |____  | (_| | | |_| | | (_) | | |_| | | |_  \__ \                     //
//                 |______|  \__,_|  \__, |  \___/   \__,_|  \__| |___/                     //
//                                    __/ |                                                 //
//                                   |___/                                                  //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

// Layout Variables
static const float mfact          = 0.5;  /* factor of master area size [0.05..0.95]      */
static const int   nmaster        = 1;    /* number of clients in master area             */
static const int   resizehints    = 1;    /* 1 means respect size hints in tiled resizals */
static const int   lockfullscreen = 1;    /* 1 will force focus on the fullscreen window  */
static const unsigned int gappih  = 10;   /* horiz inner gap between windows */
static const unsigned int gappiv  = 10;   /* vert inner gap between windows */
static const unsigned int gappoh  = 10;   /* horiz outer gap between windows and screen edge */
static const unsigned int gappov  = 10;   /* vert outer gap between windows and screen edge */
static int smartgaps              = 0;    /* 1 means no outer gap when there is only one window */

// Layouts and Layout Rules
#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
static const Layout layouts[] = {
        //  Symbol      Arrange function        Name
        {   "[]=",      tile,                   "Tile"                          },    /* first entry is default */
        {   "[M]",      monocle,                "Monocle"                       },
        {   "[@]",      spiral,                 "Spiral"                        },
        {   "[\\]",     dwindle,                "Dwindle"                       },
        {   "H[]",      deck,                   "Deck"                          },
        {   "TTT",      bstack,                 "B-Stack"                       },
        {   "===",      bstackhoriz,            "B-Stack Horizontal"            },
        {   "HHH",      grid,                   "Grid"                          },
        {   "###",      nrowgrid,               "Narrow Grid"                   },
        {   "---",      horizgrid,              "Horizontal Grid"               },
        {   ":::",      gaplessgrid,            "Gapless Grid"                  },
        {   "|M|",      centeredmaster,         "Centered Master"               },
        {   ">M>",      centeredfloatingmaster, "Centered Floating Master"      },
        {   "><>",      NULL,                   "Floating"                      },    /* no layout function means floating behavior */
};

#include "keydefs.h"

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

                                /* jdwm misc Keys: */

        //Modifier              Key             Function          Argument
        { Win,                  XK_Return,      spawn,            SHCMD( "st" ) },
        { Win,                  XK_grave,       togglescratch,    SHCMD( "st -T Scratchpad" ) },
        { Win|Shift,            XK_grave,       spawn,            SHCMD( "st -T Scratchpad" ) },
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
        //{ Win,                  PageUp,         spawn,            SHCMD( "pamixer -i 5" ) },
        //{ Win,                  PageDown,       spawn,            SHCMD( "pamixer -d 5" ) },
        //{ Win,                  XK_Scroll_Lock, spawn,            SHCMD( "playerctl next" ) },
        //{ Win,                  XK_Print,       spawn,            SHCMD( "playerctl previous" ) },
        //{ Win,                  XK_Home,        spawn,            SHCMD( "playerctl position 10+" ) },
        //{ Win,                  XK_End,         spawn,            SHCMD( "playerctl position 10-" ) },
        //{ Win,                  XK_Pause,       spawn,            SHCMD( "playerctl play-pause" ) },
        //{ Win,                  XK_Insert,      spawn,            SHCMD( "pactl set-sink-mute @DEFAULT_SINK@ toggle" ) },
        //{ Win,                  XK_Delete,      spawn,            SHCMD( "pactl set-source-mute @DEFAULT_SOURCE@ toggle" ) },

//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Special Keys: */

        // Note: I have only done some as examples, there are MANY more that can be found in
        // keydefs.h / keydefs.def.h. There are hundreds of unbound keys that exist there,
        // if your keyboard has them. 

        //Modifier      Key                         Function    Argument
        //{ 0,            XK_AudioRaiseVolume,        spawn,      SHCMD("pamixer -i 5" ) },                  
        //{ 0,            XK_AudioLowerVolume,        spawn,      SHCMD("pamixer -d 5" ) },
        //{ 0,            XK_AudioNext,               spawn,      SHCMD("playerctl next" ) },
        //{ 0,            XK_AudioPrev,               spawn,      SHCMD("playerctl previous") },
        //{ 0,            XK_AudioPlay,               spawn,      SHCMD("playerctl play") },
        //{ 0,            XK_AudioStop,               spawn,      SHCMD("playerctl pause") },
        //{ 0,            XK_AudioRewind,             spawn,      SHCMD("playerctl position 10-") },
        //{ 0,            XK_AudioForward,            spawn,      SHCMD("playerctl position 10+") },
        //{ 0,            XK_AudioMute,               spawn,      SHCMD("pactl set-sink-mute @DEFAULT_SINK@ toggle") },
        //{ 0,            XK_AudioMicMute,            spawn,      SHCMD("pactl set-source-mute @DEFAULT_SOURCE@ toggle") },
        //{ 0,            XK_Sleep,                   spawn,      SHCMD("systemctl suspend") }, 
        //{ 0,            XK_Standby,                 spawn,      SHCMD("systemctl suspend") },
        //{ 0,            XK_Suspend,                 spawn,      SHCMD("systemctl suspend") },
        //{ 0,            XK_Hibernate,               spawn,      SHCMD("systemctl hibernate") },

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


// Click can be Tags, LayoutSymbol, StatusText, WindowTitle, ClientWindow, or Desktop
static const Button buttons[] = {

//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Left Mouse Button: */

        //Click           Key            Button          Function        Argument 
        { LayoutSymbol,   0,             LeftClick,      setlayout,      {0} },
        { Tags,           0,             LeftClick,      view,           {0} },
        { ClientWindow,  Win,            LeftClick,      movemouse,      {0} },
        { ClientWindow,  Win|Control,    LeftClick,      floatandmove,   {0} },

        // Used to send focus and the window to a new monitor.
        // Is it simple? Yes. Is it lazy? Yes. Is it the best option? Maybe, but im lazy.
        { Tags,          Win,            LeftClick,      tag,            {0} },
        { Tags,          Win,            LeftClick,      view,           {0} },

        


//////////////////////////////////////////////////////////////////////////////////////////////

                                /* Middle Mouse Button: */

        //Click          Key             Button           Function        Argument
        { WindowTitle,   0,              MiddleClick,     spawn,          SHCMD( "st" ) },
        { ClientWindow,  Win,            MiddleClick,     togglefloating, {0} },


//////////////////////////////////////////////////////////////////////////////////////////////

                               /* Right Mouse Button: */

        //Click          Key             Button           Function        Argument
        { Tags,          0,              RightClick,      toggleview,     {0} },
        { ClientWindow,  Win,            RightClick,      resizemouse,    {0} },

};
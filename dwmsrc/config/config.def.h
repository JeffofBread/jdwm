//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//          /\                                                                              //
//         /  \     _ __    _ __     ___    __ _   _ __    __ _   _ __     ___    ___       //
//        / /\ \   | '_ \  | '_ \   / _ \  / _` | | '__|  / _` | | '_ \   / __|  / _ \      //
//       / ____ \  | |_) | | |_) | |  __/ | (_| | | |    | (_| | | | | | | (__  |  __/      //
//      /_/    \_\ | .__/  | .__/   \___|  \__,_| |_|     \__,_| |_| |_|  \___|  \___|      //
//                 | |     | |                                                              //
//                 |_|     |_|                                                              //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

static const unsigned int borderpx              = 0;            /* border pixel of windows */
static const unsigned int fborderpx             = 5;            /* border pixel of floating windows */
static const unsigned int raisefloatwinfoc      = 1;            /* whether to raise floating window on focus, mainly to help deal with overlapping windows */
static const unsigned int snap                  = 32;           /* snap pixel */
static const unsigned int systraypinning        = 0;            /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft         = 0;            /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing        = 3;            /* systray spacing */
static const int systraypinningfailfirst        = 1;            /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray                    = 1;            /* 0 means no systray */
static const unsigned int gappih                = 10;           /* horiz inner gap between windows */
static const unsigned int gappiv                = 10;           /* vert inner gap between windows */
static const unsigned int gappoh                = 10;           /* horiz outer gap between windows and screen edge */
static const unsigned int gappov                = 10;           /* vert outer gap between windows and screen edge */
static       int smartgaps                      = 0;            /* 1 means no outer gap when there is only one window */
static const int showbar                        = 1;            /* 0 means no bar */
static const int topbar                         = 1;            /* 0 means bottom bar */
static const int showtitle                      = 1;            /* 0 means no title */
static const int showtags                       = 1;            /* 0 means no tags */
static const int showlayout                     = 1;            /* 0 means no layout indicator */
static const int showstatus                     = 1;            /* 0 means no status bar */
static const int showfloating                   = 1;            /* 0 means no floating indicator */
static const int vertpad                        = 10;           /* vertical padding of bar */
static const int sidepad                        = 10;           /* horizontal padding of bar */
static const int horizpadbar                    = 0;            /* horizontal padding for statusbar */
static const int vertpadbar                     = 2;            /* vertical padding for statusbar */
static const int ulineall                       = 0;            /* 1 to show underline on all tags, 0 for just the active ones */
static const unsigned int ulinepad              = 5;            /* horizontal padding between the underline and tag */
static const unsigned int ulinestroke           = 2;            /* thickness / height of the underline */
static const unsigned int ulinevoffset          = 1;            /* how far above the bottom of the bar the line should appear */
static const unsigned int centeredwindowname    = 1;            /* 0 is default dwm behavior, 1 centers the name on the monitor width (not the bars), and over 1 is a lazy toggle off for the window name */
#define ICONSIZE                                  20            /* icon size */
#define ICONSPACING                               5             /* space between icon and title */

// Fonts
static const char *fonts[]                      = { "JetBrainsMono:size=14" };
static const char dmenufont[]                   =   "JetBrainsMono:size=14";

// Include chosen theme here. Make custom themes by copying .def file and customizing/renaming
#include <jeffs_theme.h>
//#include <default_theme.h>

// Sticky Icon
static const XPoint stickyicon[]        = { {0,0}, {4,0}, {4,8}, {2,6}, {0,8}, {0,0} }; /* represents the icon as an array of vertices */
static const XPoint stickyiconbb        = {4,8}; /* defines the bottom right corner of the polygon's bounding box (speeds up scaling) */

// Tags
static const char *tags[]               = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const char *tagsalt[]            = { "a", "b", "c", "d", "e", "f", "g", "h", "i" };
static const int   momentaryalttags     = 0; /* 1 means alttags will show only when key is held down*/

// Window Swallowing Variables
static const int  swaldecay             = 3;
static const int  swalretroactive       = 1;
static const char swalsymbol[]          = "🔗";


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
static const int   decorhints     = 1;    /* 1 means respect decoration hints             */
static const int   lockfullscreen = 1;    /* 1 will force focus on the fullscreen window  */

// Layouts and Layout Rules
#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include <vanitygaps.c>
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

// Window Rules
static const Rule rules[] = {

        /* xprop(1):
         *      WM_CLASS(STRING) = instance, class
         *      WM_NAME(STRING) = title
         */

        // Class        Instance    Title       Tags Mask       isfloating      Monitor         ignoretransient
        { "Gimp",       NULL,       NULL,       0,              0,              -1,             0 },
        { "Firefox",    NULL,       NULL,       1 << 8,         0,              -1,             0 },
        { "Rofi",       NULL,       NULL,       0,              1,              -1,             0 },
};


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

// IPC Variables
static const char *ipcsockpath = "/tmp/dwm.sock";
static IPCCommand ipccommands[] = {
  IPCCOMMAND(  view,            1,  {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggleview,      1,  {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tag,             1,  {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggletag,       1,  {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tagmon,          1,  {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  focusmon,        1,  {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  focusstack,      1,  {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  zoom,            1,  {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  incnmaster,      1,  {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  killclient,      1,  {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  togglefloating,  1,  {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  setmfact,        1,  {ARG_TYPE_FLOAT}  ),
  IPCCOMMAND(  setlayoutsafe,   1,  {ARG_TYPE_PTR}    ),
  IPCCOMMAND(  quit,            1,  {ARG_TYPE_NONE}   )
};

// Helper for spawning shell commands in the pre dwm-5.0 fashion
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

// Commands
static const char *termcmd[]            = { "kitty", NULL };
static const char *recompilecmd[]       = { "jeff_dwm-recompile.sh", NULL };
static const char  scratchpadname[]     =   "Scratchpad";
static const char *scratchpadcmd[]      = { "kitty", "-T", scratchpadname, NULL };
static const char *rofi_launcher_cmd[]  = { "rofi", "-no-fixed-num-lines", /*"-normal-window",*/ "-show", "drun", };
static const char *layoutmenucmd[]      = { "rofi_layoutmenu.sh", NULL };
static const char *alttabcmd[]          = { "rofi", "-no-fixed-num-lines", /*"-normal-window",*/ "-show", "window", NULL };

// Function Includes
#include <autorun.h>
#include <focusurgent.c>
#include <shift-tools.c>

// Include Button, Key Bindings, and Key Defenitions
#include <keydefs.h>
#include <binds.h>
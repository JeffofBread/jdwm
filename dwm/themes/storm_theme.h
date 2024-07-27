static const char darkblue[]    = "#16162F";
static const char midblue[]     = "#418EB4";
static const char lightblue[]   = "#A8CAD3"; 
static const char nearblack[]   = "#0B0D1E";
static const char lightpurple[] = "#645A6D";
static const char orange[]      = "#fa9061";
static const char white[]       = "#ffffff";

static const char *colors[][4] = {
    //               fg             bg              border          float
    [SchemeNorm] = { nearblack,     nearblack,      darkblue,       darkblue },     // Generic scheme
    [SchemeSel]  = { nearblack,     nearblack,      lightblue,      lightblue },    // Generic scheme for focused windows

    // colorbar Patch
    [SchemeSystray]     = { lightblue,  darkblue,   "#000000"  },   // Systray {text,background,not used but cannot be empty}
    [SchemeStatus]      = { lightblue,  darkblue,   "#000000"  },   // Statusbar right {text,background,not used but cannot be empty}
    [SchemeTagsSel]     = { orange,     darkblue,   "#000000"  },   // Tagbar left selected {text,background,not used but cannot be empty}
    [SchemeTagsNorm]    = { lightblue,  darkblue,   "#000000"  },   // Tagbar left unselected {text,background,not used but cannot be empty}
    [SchemeInfoSel]     = { lightblue,  darkblue,   "#000000"  },   // infobar middle  selected {text,background,not used but cannot be empty}
    [SchemeInfoNorm]    = { lightblue,  darkblue,   "#000000"  },   // infobar middle  unselected {text,background,not used but cannot be empty}
};


static char *wallpapercmd[] = { "feh", "--bg-fill", "/usr/local/share/jeff_dwm/dwm/themes/wallpapers/storm/storm.png", NULL, };
#define ROFITHEMEFILE storm.rasi
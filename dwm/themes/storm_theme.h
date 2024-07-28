// Storm Theme

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

// Wallpaper command executed by execvp() on any run or restart
static char *wallpapercmd[] = { "feh", "--bg-fill", "/usr/local/share/jeff_dwm/dwm/themes/wallpapers/storm/storm.png", NULL };

// Name of the rofi theme file (without path, path defined in jeff_dwm.c, should be generic)
#define ROFITHEMEFILE storm.rasi
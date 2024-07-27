// Jeff's custom color scheme, very simple green scheme
static const char greygreen[]   = "#2f4a34";    // SchemeSel
static const char normalgreen[] = "#253a29";    // Bar background
static const char darkgreen[]   = "#111b13";    // SchemeNorm
static const char bluegreen[]   = "#1e453e";    // Unused
static const char grey[]        = "#25272a";    // Window background
static const char white[]       = "#e1e0db";    // Text


static const char *colors[][4] = {
    //               fg         bg      border      float
    [SchemeNorm] = { darkgreen, grey,   darkgreen,  darkgreen },    // Generic scheme
    [SchemeSel]  = { greygreen, grey,   greygreen,  greygreen },    // Generic scheme for focused windows

    // colorbar Patch
    [SchemeSystray]     = { white,          normalgreen,      "#000000"  },   // Systray {text,background,not used but cannot be empty}
    [SchemeStatus]      = { white,          normalgreen,      "#000000"  },   // Statusbar right {text,background,not used but cannot be empty}
    [SchemeTagsSel]     = { white,          normalgreen,      "#000000"  },   // Tagbar left selected {text,background,not used but cannot be empty}
    [SchemeTagsNorm]    = { white,          normalgreen,      "#000000"  },   // Tagbar left unselected {text,background,not used but cannot be empty}
    [SchemeInfoSel]     = { white,          normalgreen,      "#000000"  },   // infobar middle  selected {text,background,not used but cannot be empty}
    [SchemeInfoNorm]    = { white,          normalgreen,      "#000000"  },   // infobar middle  unselected {text,background,not used but cannot be empty}
};

static char *wallpapercmd[] = { "feh", "--bg-fill", "/usr/local/share/jeff_dwm/dwm/themes/wallpapers/lush/lush1.jpg", "/usr/local/share/jeff_dwm/dwm/themes/wallpapers/lush/lush2.jpg", NULL, };
#define ROFITHEMEFILE lush.rasi
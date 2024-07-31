// 🌲 Lush Theme

// Lush green theme created by @JeffOfBread
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

// Wallpaper command executed by execvp() on any run or restart
static char *wallpapercmd[] = { "feh", "--bg-fill", "/usr/local/share/jdwm/dwm/themes/wallpapers/lush/lush1.jpg", "/usr/local/share/jdwm/dwm/themes/wallpapers/lush/lush2.jpg", NULL };

// Name of the rofi theme file (without path, path defined in jdwm.c, should be generic)
#define ROFITHEMEFILE lush.rasi
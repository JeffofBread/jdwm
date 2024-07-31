// 📦 Default Theme

// Default-ish dwm colorscheme created by the suckless team, adapted to by @JeffOfBread
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

// Wallpaper command executed by execvp() on any run or restart
char *wallpapercmd[] = { "feh", "--bg-fill", "/usr/local/share/jdwm/dwm/themes/wallpapers/default/grey.jpg", NULL };

// Name of the rofi theme file (without path, path defined in jdwm.c, should be generic)
#define ROFITHEMEFILE default.rasi
// For autostarting programs that don't persist though a dwm restart, like most tray apps
static const char *const autostart[] = {
	"dwmblocks", NULL,
	"example", "-e", "--example" NULL,
    "example2", "-e", "--example" NULL,
	NULL /* terminate */
};
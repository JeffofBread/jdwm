// For autostarting programs that DON'T persist though a dwm restart, like some tray apps
static const char *const startandrestart[] = {
	"dwmblocks", NULL, // Dont remove unless you want to lose functionality
	//"example", "-e", "--example" NULL,
    //"example2", "-e", "--example" NULL,
	NULL /* terminate */
};

// For autostarting programs that DO persist though a dwm restart, compositor or a favorite desktop app
static const char *const startonce[] = {
	"jeff_dwm_setup.sh", NULL,
	"picom", NULL,
	//"example", "-e", "--example" NULL,
    //"example2", "-e", "--example" NULL,
	NULL
};
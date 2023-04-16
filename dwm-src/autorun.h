// For autostarting programs that don't persist though a dwm restart, like most tray apps
static const char *const autostart[] = {
	"pasystray", NULL,
	"clipit", NULL,
	"blueman-applet", NULL,
	"nm-applet", NULL,
	"solaar", "-w", "hide", NULL,
	"jetbrains-toolbox", "--minimize", NULL,
	"steam", "-silent", NULL,
	NULL /* terminate */
};
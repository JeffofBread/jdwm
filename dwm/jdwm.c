#include <X11/X.h>
#include <argp.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

/* macros */
#define APPEND_(x, y) #x #y
#define APPEND(x, y) APPEND_(x, y)
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)                     \
	(mask & ~(numlockmask | LockMask) & \
	 (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))
#define INTERSECT(x, y, w, h, m)                                         \
	(MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) * \
	 MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))
#define ISVISIBLE(C) ((C->tags & C->mon->tagset[C->mon->seltags]) || C->issticky)
#define LENGTH(X) (sizeof X / sizeof X[0])
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)
#define WIDTH(X) ((X)->w + 2 * (X)->bw)
#define HEIGHT(X) ((X)->h + 2 * (X)->bw)
#define TAGMASK ((1 << LENGTH(tags)) - 1)
#define TAGSLENGTH (LENGTH(tags))
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + lrpad)
#define ColFloat 3
#define SHCMD(cmd)                                    \
	{                                             \
		.v = (const char *[])                 \
		{                                     \
			"/bin/bash", "-ic", cmd, NULL \
		}                                     \
	}
#define SYSTEM_TRAY_REQUEST_DOCK 0

// Define various paths
// clang-format off
#define ROFITHEME APPEND(ROFITHEMEDIR, ROFITHEMEFILE)
// clang-format on

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_FOCUS_IN 4
#define XEMBED_MODALITY_ON 10
#define XEMBED_MAPPED (1 << 0)
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_WINDOW_DEACTIVATE 2
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum {
	SchemeNorm,
	SchemeSel,
	SchemeSystray,
	SchemeStatus,
	SchemeTagsSel,
	SchemeTagsNorm,
	SchemeInfoSel,
	SchemeInfoNorm
}; /* color schemes */
enum {
	NetSupported,
	NetWMName,
	NetWMIcon,
	NetWMState,
	NetWMCheck,
	NetSystemTray,
	NetSystemTrayOP,
	NetSystemTrayOrientation,
	NetSystemTrayOrientationHorz,
	NetWMFullscreen,
	NetWMSticky,
	NetActiveWindow,
	NetWMWindowType,
	NetWMWindowTypeDialog,
	NetClientList,
	NetClientInfo,
	NetDesktopNames,
	NetDesktopViewport,
	NetNumberOfDesktops,
	NetCurrentDesktop,
	NetWMWindowTypeDesktop,
	NetLast
}; /* EWMH atoms */
enum { Manager, Xembed, XembedInfo, XLast }; /* Xembed atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

typedef struct TagState TagState;
struct TagState {
	int selected;
	int occupied;
	int urgent;
};

typedef struct ClientState ClientState;
struct ClientState {
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen, ignoretransient, issticky,
		fakefullscreen, issteam, hintsvalid;
};

typedef union {
	long i;
	unsigned long ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
	char name[256];
	unsigned int tags;
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh, oldfx, oldfy, oldfw, oldfh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen, ignoretransient, issticky,
		fakefullscreen, issteam, hintsvalid;
	unsigned int icw, ich;
	ClientState prevstate;
	Client *next;
	Client *snext;
	Monitor *mon;
	Window win;
	Picture icon;
};

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
	const char *name;
} Layout;

typedef struct Pertag Pertag;
struct Monitor {
	char ltsymbol[16];
	char lastltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by; /* bar geometry */
	int mx, my, mw, mh; /* screen size */
	int wx, wy, ww, wh; /* window area  */
	int gappih; /* horizontal gap between windows */
	int gappiv; /* vertical gap between windows */
	int gappoh; /* horizontal outer gaps */
	int gappov; /* vertical outer gaps */
	int enablegaps; /* enable gaps per tag */
	int showbar, topbar;
	int ltcur;
	unsigned int seltags, sellt, tagset[2];
	const Layout *lt[2], *lastlt;
	TagState tagstate;
	Client *clients;
	Client *sel;
	Client *lastsel;
	Client *stack;
	Monitor *next;
	Window barwin;
	Pertag *pertag;
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int monitor;
	int ignoretransient;
	int unmanaged;
} Rule;

typedef struct Systray Systray;
struct Systray {
	Window win;
	Client *icons;
};

struct arguments {
	char *args[1];
};

/* function declarations */
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void bstack(Monitor *m);
static void bstackhoriz(Monitor *m);
static void buttonpress(XEvent *e);
static void centeredfloatingmaster(Monitor *m);
static void centeredmaster(Monitor *m);
static void centerfloating(const Arg *arg);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(void);
static void deck(Monitor *m);
static void defaultgaps(const Arg *arg);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static void dwindle(Monitor *m);
static void enqueue(Client *c);
static void enqueuestack(Client *c);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void fibonacci(Monitor *m, int s);
static void floatandmove(const Arg *arg);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static void freeicon(Client *c);
static void fullscreencheck(Client *c);
static void gaplessgrid(Monitor *m);
static Atom getatomprop(Client *c, Atom prop);
static void getfacts(Monitor *m, int msize, int ssize, float *mf, float *sf, int *mr, int *sr);
static void getgaps(Monitor *m, int *oh, int *ov, int *ih, int *iv, unsigned int *nc);
static Picture geticonprop(Window w, unsigned int *icw, unsigned int *ich);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static unsigned int getsystraywidth(void);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void grid(Monitor *m);
static int handlexevent(struct epoll_event *ev);
static void horizgrid(Monitor *m);
static void incnmaster(const Arg *arg);
static void incrgaps(const Arg *arg);
static void incrigaps(const Arg *arg);
static void incrihgaps(const Arg *arg);
static void incrivgaps(const Arg *arg);
static void incrogaps(const Arg *arg);
static void incrohgaps(const Arg *arg);
static void incrovgaps(const Arg *arg);
static void keypress(XEvent *e);
static void killclient(const Arg *arg);
static void layoutscroll(const Arg *arg);
static void losefullscreen(Client *next);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void nrowgrid(Monitor *m);
static error_t parse_opt(int key, char *arg, struct argp_state *state);
static void pop(Client *c);
static uint32_t prealpha(uint32_t p);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void removesystrayicon(Client *i);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizebarwin(Monitor *m);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void resizerequest(XEvent *e);
static void restack(Monitor *m);
static void rotatestack(const Arg *arg);
static void run(void);
static void scan(void);
static int sendevent(Window w, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setclienttagprop(Client *c);
static void setcurrentdesktop(void);
static void setdesktopnames(void);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setgaps(int oh, int ov, int ih, int iv);
static void setlayout(const Arg *arg);
static void setlayoutsafe(const Arg *arg);
static void setmfact(const Arg *arg);
static void setnumdesktops(void);
static void setsticky(Client *c, int sticky);
static void setup(void);
static void setupepoll(void);
static void seturgent(Client *c, int urg);
static void setviewport(void);
static void shiftboth(const Arg *arg);
static void shiftswaptags(const Arg *arg);
static void shifttag(const Arg *arg);
static void shifttagclients(const Arg *arg);
static void shiftview(const Arg *arg);
static void shiftviewclients(const Arg *arg);
static void showhide(Client *c);
static void sigchld(int unused);
static void sighup(int unused);
static void sigterm(int unused);
static void spawn(const Arg *arg);
static void spiral(Monitor *m);
static void swaptags(const Arg *arg);
static Monitor *systraytomon(Monitor *m);
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void tile(Monitor *m);
static void togglebar(const Arg *arg);
static void togglefakefullscreen(const Arg *arg);
static void togglefloating(const Arg *arg);
static void togglefullscreen(const Arg *arg);
static void togglegaps(const Arg *arg);
static void togglescratch(const Arg *arg);
static void togglesticky(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updateclientlist(void);
static void updatecurrentdesktop(void);
static int updategeom(void);
static void updateicon(Client *c);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatesystray(void);
static void updatesystrayicongeom(Client *i, int w, int h);
static void updatesystrayiconstate(Client *i, XPropertyEvent *ev);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static void viewall(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static Client *wintosystrayicon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void zoom(const Arg *arg);

/* variables */
static Client *lastfocused = NULL;
static Systray *systray = NULL;
static const char broken[] = "broken";
static char stext[256];
static int screen;
static int sw, sh; /* X display screen geometry width, height */
static int bh; /* bar height */
static int lrpad; /* sum of left and right padding for text */
static int vp; /* vertical padding for bar */
static int sp; /* side padding for bar */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent])(XEvent *) = { [ButtonPress] = buttonpress,
						[ClientMessage] = clientmessage,
						[ConfigureRequest] = configurerequest,
						[ConfigureNotify] = configurenotify,
						[DestroyNotify] = destroynotify,
						[EnterNotify] = enternotify,
						[Expose] = expose,
						[FocusIn] = focusin,
						[KeyPress] = keypress,
						[MappingNotify] = mappingnotify,
						[MapRequest] = maprequest,
						[MotionNotify] = motionnotify,
						[PropertyNotify] = propertynotify,
						[ResizeRequest] = resizerequest,
						[UnmapNotify] = unmapnotify };
static Atom wmatom[WMLast], netatom[NetLast], xatom[XLast];
static int epoll_fd;
static int dpy_fd;
static int restart = 0;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon, *lastselmon;
static Window root, wmcheckwin;
static int firstrun = 0;
static int unmanaged = 0; /* whether the window manager should manage the new window or not */

// IPC
#include "ipc.h"

// Tertiary config file
#include <config.h>

// Sub config files
#include <keydefs.h>
#include <binds.h>
#include <autorun.h>

// IPC
static const char *ipcsockpath = "/tmp/jdwm.sock";
static IPCCommand ipccommands[] = { IPCCOMMAND(view, 1, { ARG_TYPE_UINT }),
				    IPCCOMMAND(toggleview, 1, { ARG_TYPE_UINT }),
				    IPCCOMMAND(tag, 1, { ARG_TYPE_UINT }),
				    IPCCOMMAND(toggletag, 1, { ARG_TYPE_UINT }),
				    IPCCOMMAND(tagmon, 1, { ARG_TYPE_UINT }),
				    IPCCOMMAND(focusmon, 1, { ARG_TYPE_SINT }),
				    IPCCOMMAND(focusstack, 1, { ARG_TYPE_SINT }),
				    IPCCOMMAND(zoom, 1, { ARG_TYPE_NONE }),
				    IPCCOMMAND(incnmaster, 1, { ARG_TYPE_SINT }),
				    IPCCOMMAND(killclient, 1, { ARG_TYPE_SINT }),
				    IPCCOMMAND(togglefloating, 1, { ARG_TYPE_NONE }),
				    IPCCOMMAND(setmfact, 1, { ARG_TYPE_FLOAT }),
				    IPCCOMMAND(setlayoutsafe, 1, { ARG_TYPE_PTR }),
				    IPCCOMMAND(quit, 1, { ARG_TYPE_NONE }) };
#include "IPCClient.c"
#include "yajl_dumps.c"
#include "ipc.c"

struct Pertag {
	unsigned int curtag, prevtag; /* current and previous tag */
	int nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
	float mfacts[LENGTH(tags) + 1]; /* mfacts per tag */
	unsigned int sellts[LENGTH(tags) + 1]; /* selected layouts */
	const Layout *ltidxs[LENGTH(tags) + 1][2]; /* matrix of tags and layouts indexes  */
	int showbars[LENGTH(tags) + 1]; /* display bar for the current tag */
	int enablegaps[LENGTH(tags) + 1]; /* vanitygaps */
};

static unsigned int scratchtag = 1 << LENGTH(tags);

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags {
	char limitexceeded[LENGTH(tags) > 31 ? -1 : 1];
};

/* function implementations */
void applyrules(Client *c)
{
	const char *class, *instance;
	unsigned int i;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->isfloating = 0;
	c->tags = 0;

	// Sets default values related to floating windows
	c->oldfw = c->mon->mw / 1.5;
	c->oldfh = c->mon->mh / 1.5;
	c->oldfx = c->mon->mx + (c->mon->mw / 2) - (c->oldfw / 2);
	c->oldfy = c->mon->my + (c->mon->mh / 2) - (c->oldfh / 2);

	XGetClassHint(dpy, c->win, &ch);
	class = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name ? ch.res_name : broken;

	if (strstr(class, "Steam") || strstr(class, "steam_app_")) c->issteam = 1;

	for (i = 0; i < LENGTH(rules); i++) {
		r = &rules[i];
		if ((!r->title || strstr(c->name, r->title)) && (!r->class || strstr(class, r->class)) &&
		    (!r->instance || strstr(instance, r->instance))) {
			c->ignoretransient = r->ignoretransient;
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
			unmanaged = r->unmanaged;
			for (m = mons; m && m->num != r->monitor; m = m->next);
			if (m) c->mon = m;
		}
	}
	if (ch.res_class) XFree(ch.res_class);
	if (ch.res_name) XFree(ch.res_name);
	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	Monitor *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if (interact) {
		if (*x > sw) *x = sw - WIDTH(c);
		if (*y > sh) *y = sh - HEIGHT(c);
		if (*x + *w + 2 * c->bw < 0) *x = 0;
		if (*y + *h + 2 * c->bw < 0) *y = 0;
	} else {
		if (*x >= m->wx + m->ww) *x = m->wx + m->ww - WIDTH(c);
		if (*y >= m->wy + m->wh) *y = m->wy + m->wh - HEIGHT(c);
		if (*x + *w + 2 * c->bw <= m->wx) *x = m->wx;
		if (*y + *h + 2 * c->bw <= m->wy) *y = m->wy;
	}
	if (*h < bh) *h = bh;
	if (*w < bh) *w = bh;
	if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		if (!c->hintsvalid) updatesizehints(c);
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if (baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if (c->incw) *w -= *w % c->incw;
		if (c->inch) *h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if (c->maxw) *w = MIN(*w, c->maxw);
		if (c->maxh) *h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void arrange(Monitor *m)
{
	if (m)
		showhide(m->stack);
	else
		for (m = mons; m; m = m->next) showhide(m->stack);
	if (m) {
		arrangemon(m);
		restack(m);
	} else
		for (m = mons; m; m = m->next) arrangemon(m);
}

void arrangemon(Monitor *m)
{

// Added because im lazy and dont want deal with the error, causes no known issues
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
#pragma GCC diagnostic pop
	if (m->lt[m->sellt]->arrange) m->lt[m->sellt]->arrange(m);
}

void attach(Client *c)
{
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void attachstack(Client *c)
{
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void bstack(Monitor *m)
{
	unsigned int i, n;
	int oh, ov, ih, iv;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	float mfacts, sfacts;
	int mrest, srest;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	sx = mx = m->wx + ov;
	sy = my = m->wy + oh;
	sh = mh = m->wh - 2 * oh;
	mw = m->ww - 2 * ov - iv * (MIN(n, m->nmaster) - 1);
	sw = m->ww - 2 * ov - iv * (n - m->nmaster - 1);

	if (m->nmaster && n > m->nmaster) {
		sh = (mh - ih) * (1 - m->mfact);
		mh = mh - ih - sh;
		sx = mx;
		sy = my + mh + ih;
	}

	getfacts(m, mw, sw, &mfacts, &sfacts, &mrest, &srest);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		if (i < m->nmaster) {
			resize(c, mx, my, (mw / mfacts) + (i < mrest ? 1 : 0) - (2 * c->bw), mh - (2 * c->bw), 0);
			mx += WIDTH(c) + iv;
		} else {
			resize(c, sx, sy, (sw / sfacts) + ((i - m->nmaster) < srest ? 1 : 0) - (2 * c->bw),
			       sh - (2 * c->bw), 0);
			sx += WIDTH(c) + iv;
		}
	}
}

void bstackhoriz(Monitor *m)
{
	unsigned int i, n;
	int oh, ov, ih, iv;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	float mfacts, sfacts;
	int mrest, srest;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	sx = mx = m->wx + ov;
	sy = my = m->wy + oh;
	mh = m->wh - 2 * oh;
	sh = m->wh - 2 * oh - ih * (n - m->nmaster - 1);
	mw = m->ww - 2 * ov - iv * (MIN(n, m->nmaster) - 1);
	sw = m->ww - 2 * ov;

	if (m->nmaster && n > m->nmaster) {
		sh = (mh - ih) * (1 - m->mfact);
		mh = mh - ih - sh;
		sy = my + mh + ih;
		sh = m->wh - mh - 2 * oh - ih * (n - m->nmaster);
	}

	getfacts(m, mw, sh, &mfacts, &sfacts, &mrest, &srest);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		if (i < m->nmaster) {
			resize(c, mx, my, (mw / mfacts) + (i < mrest ? 1 : 0) - (2 * c->bw), mh - (2 * c->bw), 0);
			mx += WIDTH(c) + iv;
		} else {
			resize(c, sx, sy, sw - (2 * c->bw),
			       (sh / sfacts) + ((i - m->nmaster) < srest ? 1 : 0) - (2 * c->bw), 0);
			sy += HEIGHT(c) + ih;
		}
	}
}

void buttonpress(XEvent *e)
{
	unsigned int i, x, click;
	Arg arg = { 0 };
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev = &e->xbutton;

	click = ClkRootWin;
	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	if (ev->window == selmon->barwin) {
		i = x = 0;
		unsigned int occ = 0;
		for (c = m->clients; c; c = c->next) occ |= c->tags;
		do {
			/* Do not reserve space for vacant tags */
			if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i)) continue;
			if (showtags) x += TEXTW(tags[i]);
		} while (ev->x >= x && ++i < LENGTH(tags));
		if (i < LENGTH(tags) && showtags) {
			click = ClkTagBar;
			arg.ui = 1 << i;
		} else if (ev->x < x + TEXTW(selmon->ltsymbol) && showlayout)
			click = ClkLtSymbol;
		else if (ev->x > selmon->ww - (int)TEXTW(stext) - getsystraywidth() && showstatus)
			click = ClkStatusText;
		else if (showtitle)
			click = ClkWinTitle;
	} else if ((c = wintoclient(ev->window))) {
		focus(c);
		restack(selmon);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}
	for (i = 0; i < LENGTH(buttons); i++)
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button &&
		    CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void centeredfloatingmaster(Monitor *m)
{
	unsigned int i, n;
	float mfacts, sfacts;
	float mivf = 1.0; // master inner vertical gap factor
	int oh, ov, ih, iv, mrest, srest;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	sx = mx = m->wx + ov;
	sy = my = m->wy + oh;
	sh = mh = m->wh - 2 * oh;
	mw = m->ww - 2 * ov - iv * (n - 1);
	sw = m->ww - 2 * ov - iv * (n - m->nmaster - 1);

	if (m->nmaster && n > m->nmaster) {
		mivf = 0.8;
		/* go mfact box in the center if more than nmaster clients */
		if (m->ww > m->wh) {
			mw = m->ww * m->mfact - iv * mivf * (MIN(n, m->nmaster) - 1);
			mh = m->wh * 0.9;
		} else {
			mw = m->ww * 0.9 - iv * mivf * (MIN(n, m->nmaster) - 1);
			mh = m->wh * m->mfact;
		}
		mx = m->wx + (m->ww - mw) / 2;
		my = m->wy + (m->wh - mh - 2 * oh) / 2;

		sx = m->wx + ov;
		sy = m->wy + oh;
		sh = m->wh - 2 * oh;
	}

	getfacts(m, mw, sw, &mfacts, &sfacts, &mrest, &srest);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			/* nmaster clients are stacked horizontally, in the center of the screen */
			resize(c, mx, my, (mw / mfacts) + (i < mrest ? 1 : 0) - (2 * c->bw), mh - (2 * c->bw), 0);
			mx += WIDTH(c) + iv * mivf;
		} else {
			/* stack clients are stacked horizontally */
			resize(c, sx, sy, (sw / sfacts) + ((i - m->nmaster) < srest ? 1 : 0) - (2 * c->bw),
			       sh - (2 * c->bw), 0);
			sx += WIDTH(c) + iv;
		}
}

void centeredmaster(Monitor *m)
{
	unsigned int i, n;
	int oh, ov, ih, iv;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int lx = 0, ly = 0, lw = 0, lh = 0;
	int rx = 0, ry = 0, rw = 0, rh = 0;
	float mfacts = 0, lfacts = 0, rfacts = 0;
	int mtotal = 0, ltotal = 0, rtotal = 0;
	int mrest = 0, lrest = 0, rrest = 0;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	/* initialize areas */
	mx = m->wx + ov;
	my = m->wy + oh;
	mh = m->wh - 2 * oh - ih * ((!m->nmaster ? n : MIN(n, m->nmaster)) - 1);
	mw = m->ww - 2 * ov;
	lh = m->wh - 2 * oh - ih * (((n - m->nmaster) / 2) - 1);
	rh = m->wh - 2 * oh - ih * (((n - m->nmaster) / 2) - ((n - m->nmaster) % 2 ? 0 : 1));

	if (m->nmaster && n > m->nmaster) {
		/* go mfact box in the center if more than nmaster clients */
		if (n - m->nmaster > 1) {
			/* ||<-S->|<---M--->|<-S->|| */
			mw = (m->ww - 2 * ov - 2 * iv) * m->mfact;
			lw = (m->ww - mw - 2 * ov - 2 * iv) / 2;
			rw = (m->ww - mw - 2 * ov - 2 * iv) - lw;
			mx += lw + iv;
		} else {
			/* ||<---M--->|<-S->|| */
			mw = (mw - iv) * m->mfact;
			lw = 0;
			rw = m->ww - mw - iv - 2 * ov;
		}
		lx = m->wx + ov;
		ly = m->wy + oh;
		rx = mx + mw + iv;
		ry = m->wy + oh;
	}

	/* calculate facts */
	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++) {
		if (!m->nmaster || n < m->nmaster)
			mfacts += 1;
		else if ((n - m->nmaster) % 2)
			lfacts += 1; // total factor of left hand stack area
		else
			rfacts += 1; // total factor of right hand stack area
	}

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++)
		if (!m->nmaster || n < m->nmaster)
			mtotal += mh / mfacts;
		else if ((n - m->nmaster) % 2)
			ltotal += lh / lfacts;
		else
			rtotal += rh / rfacts;

	mrest = mh - mtotal;
	lrest = lh - ltotal;
	rrest = rh - rtotal;

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		if (!m->nmaster || i < m->nmaster) {
			/* nmaster clients are stacked vertically, in the center of the screen */
			resize(c, mx, my, mw - (2 * c->bw), (mh / mfacts) + (i < mrest ? 1 : 0) - (2 * c->bw), 0);
			my += HEIGHT(c) + ih;
		} else {
			/* stack clients are stacked vertically */
			if ((i - m->nmaster) % 2) {
				resize(c, lx, ly, lw - (2 * c->bw),
				       (lh / lfacts) + ((i - 2 * m->nmaster) < 2 * lrest ? 1 : 0) - (2 * c->bw), 0);
				ly += HEIGHT(c) + ih;
			} else {
				resize(c, rx, ry, rw - (2 * c->bw),
				       (rh / rfacts) + ((i - 2 * m->nmaster) < 2 * rrest ? 1 : 0) - (2 * c->bw), 0);
				ry += HEIGHT(c) + ih;
			}
		}
	}
}

void centerfloating(const Arg *arg)
{
	Client *c = selmon->sel;
	int nx, ny;

	if (c->isfloating) {
		nx = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
		ny = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;

		resize(c, nx, ny, c->w, c->h, 0);
		XWarpPointer(dpy, None, c->mon->sel->win, 0, 0, 0, 0, c->mon->sel->w / 2, c->mon->sel->h / 2);
	}
}

void checkotherwm(void)
{
	xerrorxlib = XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void cleanup(void)
{
	Arg a = { .ui = ~0 };
	Layout foo = { "", NULL };
	Monitor *m;
	size_t i;

	view(&a);
	selmon->lt[selmon->sellt] = &foo;
	for (m = mons; m; m = m->next)
		while (m->stack) unmanage(m->stack, 0);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	while (mons) {
		cleanupmon(mons);
	}
	if (showsystray) {
		XUnmapWindow(dpy, systray->win);
		XDestroyWindow(dpy, systray->win);
		free(systray);
	}
	for (i = 0; i < CurLast; i++) drw_cur_free(drw, cursor[i]);
	for (i = 0; i < LENGTH(colors); i++) free(scheme[i]);
	free(scheme);
	XDestroyWindow(dpy, wmcheckwin);
	drw_free(drw);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dpy, root, netatom[NetActiveWindow]);

	ipc_cleanup();

	if (close(epoll_fd) < 0) {
		fprintf(stderr, "Failed to close epoll file descriptor\n");
	}
}

void cleanupmon(Monitor *mon)
{
	Monitor *m;

	if (mon == mons)
		mons = mons->next;
	else {
		for (m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
	}
	XUnmapWindow(dpy, mon->barwin);
	XDestroyWindow(dpy, mon->barwin);
	free(mon);
}

void clientmessage(XEvent *e)
{
	XWindowAttributes wa;
	XSetWindowAttributes swa;
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if (showsystray && cme->window == systray->win && cme->message_type == netatom[NetSystemTrayOP]) {
		/* add systray icons */
		if (cme->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
			if (!(c = (Client *)calloc(1, sizeof(Client))))
				die("fatal: could not malloc() %u bytes\n", sizeof(Client));
			if (!(c->win = cme->data.l[2])) {
				free(c);
				return;
			}
			c->mon = selmon;
			c->next = systray->icons;
			systray->icons = c;
			if (!XGetWindowAttributes(dpy, c->win, &wa)) {
				/* use sane defaults */
				wa.width = bh;
				wa.height = bh;
				wa.border_width = 0;
			}
			c->x = c->oldx = c->y = c->oldy = 0;
			c->w = c->oldw = wa.width;
			c->h = c->oldh = wa.height;
			c->oldbw = wa.border_width;
			c->bw = 0;
			c->isfloating = True;
			/* reuse tags field as mapped status */
			c->tags = 1;
			updatesizehints(c);
			updatesystrayicongeom(c, wa.width, wa.height);
			XAddToSaveSet(dpy, c->win);
			XSelectInput(dpy, c->win, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);

			/* https://www.reddit.com/r/suckless/comments/hzop00/comment/fzpr3dc/?utm_source=share&utm_medium=web2x&context=3 */
			XClassHint ch = { "dwmsystray", "dwmsystray" };
			XSetClassHint(dpy, c->win, &ch);

			XReparentWindow(dpy, c->win, systray->win, 0, 0);
			/* use parents background color */
			swa.background_pixel = scheme[SchemeSystray][ColBg].pixel;
			XChangeWindowAttributes(dpy, c->win, CWBackPixel, &swa);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0,
				  systray->win, XEMBED_EMBEDDED_VERSION);
			/* FIXME not sure if I have to send these events, too */
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_FOCUS_IN, 0,
				  systray->win, XEMBED_EMBEDDED_VERSION);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0,
				  systray->win, XEMBED_EMBEDDED_VERSION);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_MODALITY_ON, 0,
				  systray->win, XEMBED_EMBEDDED_VERSION);
			XSync(dpy, False);
			resizebarwin(selmon);
			updatesystray();
			setclientstate(c, NormalState);
		}
		return;
	}

	if (!c) return;
	if (cme->message_type == netatom[NetWMState]) {
		if (cme->data.l[1] == netatom[NetWMFullscreen] || cme->data.l[2] == netatom[NetWMFullscreen]) {
			if (c->fakefullscreen == 2 && c->isfullscreen) c->fakefullscreen = 3;
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
					  || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));

			if (cme->data.l[1] == netatom[NetWMSticky] || cme->data.l[2] == netatom[NetWMSticky])
				setsticky(c, (cme->data.l[0] == 1 || (cme->data.l[0] == 2 && !c->issticky)));
		}
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (c != selmon->sel && !c->isurgent) seturgent(c, 1);
	}
}

void configure(Client *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void configurenotify(XEvent *e)
{
	Monitor *m;
	Client *c;
	XConfigureEvent *ev = &e->xconfigure;
	int dirty;

	/* TODO: updategeom handling sucks, needs to be simplified */
	if (ev->window == root) {
		dirty = (sw != ev->width || sh != ev->height);
		sw = ev->width;
		sh = ev->height;
		if (updategeom() || dirty) {
			drw_resize(drw, sw, bh);
			updatebars();
			for (m = mons; m; m = m->next) {
				for (c = m->clients; c; c = c->next)
					if (c->isfullscreen && c->fakefullscreen != 1)
						resizeclient(c, m->mx, m->my, m->mw, m->mh);
				resizebarwin(m);
			}
			focus(NULL);
			arrange(NULL);
		}
	}
}

void configurerequest(XEvent *e)
{
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if ((c = wintoclient(ev->window))) {
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
			m = c->mon;
			if (!c->issteam) {
				if (ev->value_mask & CWX) {
					c->oldx = c->x;
					c->x = m->mx + ev->x;
				}
				if (ev->value_mask & CWY) {
					c->oldy = c->y;
					c->y = m->my + ev->y;
				}
			}
			if (ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}
			if (ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}
			if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
			if ((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			if ((ev->value_mask & (CWX | CWY)) && !(ev->value_mask & (CWWidth | CWHeight))) configure(c);
			if (ISVISIBLE(c)) XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else
			configure(c);
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

Monitor *createmon(void)
{
	Monitor *m;
	unsigned int i;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = mfact;
	m->nmaster = nmaster;
	m->showbar = showbar;
	m->enablegaps = 1;
	m->topbar = topbar;
	m->ltcur = 0;
	m->gappih = gappih;
	m->gappiv = gappiv;
	m->gappoh = gappoh;
	m->gappov = gappov;
	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % LENGTH(layouts)];
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
	m->pertag = ecalloc(1, sizeof(Pertag));
	m->pertag->curtag = m->pertag->prevtag = 1;

	for (i = 0; i <= LENGTH(tags); i++) {
		m->pertag->nmasters[i] = m->nmaster;
		m->pertag->mfacts[i] = m->mfact;

		m->pertag->ltidxs[i][0] = m->lt[0];
		m->pertag->ltidxs[i][1] = m->lt[1];
		m->pertag->sellts[i] = m->sellt;

		m->pertag->showbars[i] = m->showbar;

		m->pertag->enablegaps[i] = m->enablegaps;
	}
	return m;
}

void deck(Monitor *m)
{
	unsigned int i, n;
	int oh, ov, ih, iv;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	float mfacts, sfacts;
	int mrest, srest;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	sx = mx = m->wx + ov;
	sy = my = m->wy + oh;
	sh = mh = m->wh - 2 * oh - ih * (MIN(n, m->nmaster) - 1);
	sw = mw = m->ww - 2 * ov;

	if (m->nmaster && n > m->nmaster) {
		sw = (mw - iv) * (1 - m->mfact);
		mw = mw - iv - sw;
		sx = mx + mw + iv;
		sh = m->wh - 2 * oh;
	}

	getfacts(m, mh, sh, &mfacts, &sfacts, &mrest, &srest);

	if (n - m->nmaster > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "D %d", n - m->nmaster);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			resize(c, mx, my, mw - (2 * c->bw), (mh / mfacts) + (i < mrest ? 1 : 0) - (2 * c->bw), 0);
			my += HEIGHT(c) + ih;
		} else {
			resize(c, sx, sy, sw - (2 * c->bw), sh - (2 * c->bw), 0);
		}
}

void defaultgaps(const Arg *arg)
{
	setgaps(gappoh, gappov, gappih, gappiv);
}

void destroynotify(XEvent *e)
{
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;
	if ((c = wintosystrayicon(ev->window))) {
		removesystrayicon(c);
		resizebarwin(selmon);
		updatesystray();
	}
	if ((c = wintoclient(ev->window))) unmanage(c, 1);
}

void detach(Client *c)
{
	Client **tc;

	for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void detachstack(Client *c)
{
	Client **tc, *t;

	for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if (c == c->mon->sel) {
		for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
}

Monitor *dirtomon(int dir)
{
	Monitor *m = NULL;

	if (dir > 0) {
		if (!(m = selmon->next)) m = mons;
	} else if (selmon == mons)
		for (m = mons; m->next; m = m->next);
	else
		for (m = mons; m->next != selmon; m = m->next);
	return m;
}

void drawbar(Monitor *m)
{
	int x, w, tw = 0, stw = 0;
	int boxs = drw->fonts->h / 9;
	int boxw = drw->fonts->h / 6 + 2;
	unsigned int i, occ = 0, urg = 0;
	Client *c;

	if (!m->showbar) {
		return;
	}

	if (showsystray && m == systraytomon(m) && !systrayonleft) stw = getsystraywidth();

	/* draw status first so it can be overdrawn by tags later */
	if (m == selmon && showstatus) { /* status is only drawn on selected monitor */
		drw_setscheme(drw, scheme[SchemeStatus]);
		tw = TEXTW(stext) - lrpad / 2;
		drw_text(drw, m->ww - tw - stw - 2 * sp, 0, tw, bh, lrpad / 2, stext, 0);
	}

	resizebarwin(m);
	for (c = m->clients; c; c = c->next) {
		occ |= c->tags;
		if (c->isurgent && showtags) urg |= c->tags;
	}
	x = 0;
	for (i = 0; i < LENGTH(tags); i++) {
		/* Do not draw vacant tags */
		if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i)) continue;
		if (showtags) {
			w = TEXTW(tags[i]);
			drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeTagsSel : SchemeTagsNorm]);
			drw_text(drw, x, 0, w, bh, lrpad / 2, tags[i], urg & 1 << i);
			if (ulineall ||
			    m->tagset[m->seltags] &
				    1 << i) /* if there are conflicts, just move these lines directly underneath both 'drw_setscheme' and 'drw_text' :) */
				drw_rect(drw, x + ulinepad, bh - ulinestroke - ulinevoffset, w - (ulinepad * 2),
					 ulinestroke, 1, 0);
			if (occ & 1 << i && showfloating) {
				// idk how this fixes vacant tabs + bartoggle, but it does - JeffofBread
				drw_rect(0, 0, 0, 0, 0, m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
					 urg & 1 << i);
			}
			x += w;
		}
	}

	/* draw layout indicator if showlayout */
	if (showlayout) {
		w = TEXTW(m->ltsymbol);
		drw_setscheme(drw, scheme[SchemeTagsNorm]);
		x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);
	}

	if ((w = m->ww - tw - stw - x) > bh) {
		if (m->sel && showtitle) {
			/* fix overflow when window name is bigger than window width */
			int mid = (m->ww - (int)TEXTW(m->sel->name)) / 2 - x;
			/* make sure name will not overlap on tags even when it is very long */
			mid = mid >= lrpad / 2 ? mid : lrpad / 2;
			drw_setscheme(drw, scheme[m == selmon ? SchemeInfoSel : SchemeInfoNorm]);
			if (centeredwindowname == 0) {
				drw_text(drw, x, 0, w - 2 * sp, bh,
					 lrpad / 2 + (m->sel->icon ? m->sel->icw + ICONSPACING : 0), m->sel->name, 0);
				if (m->sel->icon)
					drw_pic(drw, x + lrpad / 2, (bh - m->sel->ich) / 2, m->sel->icw, m->sel->ich,
						m->sel->icon);
				if (m->sel->isfloating && showfloating)
					drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
				if (m->sel->issticky)
					drw_polygon(drw, x + boxs, m->sel->isfloating ? boxs * 2 + boxw : boxs,
						    stickyiconbb.x, stickyiconbb.y, boxw,
						    boxw * stickyiconbb.y / stickyiconbb.x, stickyicon,
						    LENGTH(stickyicon), Nonconvex,
						    m->sel->tags & m->tagset[m->seltags]);
			} else if (centeredwindowname == 1) {
				drw_text(drw, x, 0, w - 2 * sp, bh,
					 mid + (m->sel->icon ? m->sel->icw + ICONSPACING : 0), m->sel->name, 0);
				if (m->sel->icon)
					drw_pic(drw, x + mid, (bh - m->sel->ich) / 2, m->sel->icw, m->sel->ich,
						m->sel->icon);
				if (m->sel->isfloating && showfloating)
					drw_rect(drw, x + mid - ICONSPACING - 3, boxs, boxw, boxw, m->sel->isfixed, 0);
				if (m->sel->issticky)
					drw_polygon(drw, x + mid - ICONSPACING - 3,
						    m->sel->isfloating ? boxs * 2 + boxw : boxs, stickyiconbb.x,
						    stickyiconbb.y, boxw, boxw * stickyiconbb.y / stickyiconbb.x,
						    stickyicon, LENGTH(stickyicon), Nonconvex,
						    m->sel->tags & m->tagset[m->seltags]);
			}
		} else {
			drw_setscheme(drw, scheme[SchemeInfoNorm]);
			drw_rect(drw, x, 0, w - 2 * sp, bh, 1, 1);
		}
	}
	drw_map(drw, m->barwin, 0, 0, m->ww - stw, bh);
}

void drawbars(void)
{
	Monitor *m;

	for (m = mons; m; m = m->next) drawbar(m);
}

void dwindle(Monitor *m)
{
	fibonacci(m, 1);
}

void enqueue(Client *c)
{
	Client *l;
	for (l = c->mon->clients; l && l->next; l = l->next);
	if (l) {
		l->next = c;
		c->next = NULL;
	}
}

void enqueuestack(Client *c)
{
	Client *l;
	for (l = c->mon->stack; l && l->snext; l = l->snext);
	if (l) {
		l->snext = c;
		c->snext = NULL;
	}
}

void enternotify(XEvent *e)
{
	Client *c;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root) return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	if (m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
	} else if (!c || c == selmon->sel)
		return;
	focus(c);
}

void expose(XEvent *e)
{
	Monitor *m;
	XExposeEvent *ev = &e->xexpose;

	if (ev->count == 0 && (m = wintomon(ev->window))) {
		drawbar(m);
		if (m == selmon) updatesystray();
	}
}

void fibonacci(Monitor *m, int s)
{
	unsigned int i, n;
	int nx, ny, nw, nh;
	int oh, ov, ih, iv;
	int nv, hrest = 0, wrest = 0, r = 1;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	nx = m->wx + ov;
	ny = m->wy + oh;
	nw = m->ww - 2 * ov;
	nh = m->wh - 2 * oh;

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
		if (r) {
			if ((i % 2 && (nh - ih) / 2 <= (bh + 2 * c->bw)) ||
			    (!(i % 2) && (nw - iv) / 2 <= (bh + 2 * c->bw))) {
				r = 0;
			}
			if (r && i < n - 1) {
				if (i % 2) {
					nv = (nh - ih) / 2;
					hrest = nh - 2 * nv - ih;
					nh = nv;
				} else {
					nv = (nw - iv) / 2;
					wrest = nw - 2 * nv - iv;
					nw = nv;
				}

				if ((i % 4) == 2 && !s)
					nx += nw + iv;
				else if ((i % 4) == 3 && !s)
					ny += nh + ih;
			}

			if ((i % 4) == 0) {
				if (s) {
					ny += nh + ih;
					nh += hrest;
				} else {
					nh -= hrest;
					ny -= nh + ih;
				}
			} else if ((i % 4) == 1) {
				nx += nw + iv;
				nw += wrest;
			} else if ((i % 4) == 2) {
				ny += nh + ih;
				nh += hrest;
				if (i < n - 1) nw += wrest;
			} else if ((i % 4) == 3) {
				if (s) {
					nx += nw + iv;
					nw -= wrest;
				} else {
					nw -= wrest;
					nx -= nw + iv;
					nh += hrest;
				}
			}
			if (i == 0) {
				if (n != 1) {
					nw = (m->ww - iv - 2 * ov) - (m->ww - iv - 2 * ov) * (1 - m->mfact);
					wrest = 0;
				}
				ny = m->wy + oh;
			} else if (i == 1)
				nw = m->ww - nw - iv - 2 * ov;
			i++;
		}

		resize(c, nx, ny, nw - (2 * c->bw), nh - (2 * c->bw), False);
	}
}

void floatandmove(const Arg *arg)
{
	if (selmon->sel && selmon->sel->isfloating) {
	} else
		togglefloating(NULL);
	movemouse(arg);
}

void focus(Client *c)
{
	if (!c || !ISVISIBLE(c))
		for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
	if (selmon->sel && selmon->sel != c) {
		losefullscreen(c);
		unfocus(selmon->sel, 0);
	}
	if (c) {
		if (c->mon != selmon) selmon = c->mon;
		if (c->isurgent) seturgent(c, 0);
		detachstack(c);
		attachstack(c);
		grabbuttons(c, 1);
		/* set new focused border first to avoid flickering */
		if (c->isfloating) {
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColFloat].pixel);
			if (raisefloatwinfoc) XRaiseWindow(dpy, c->win);
		} else
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		/* lastfocused may be us if another window was unmanaged */
		if (lastfocused && lastfocused != c)
			XSetWindowBorder(dpy, lastfocused->win, scheme[SchemeNorm][ColBorder].pixel);
		setfocus(c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	selmon->sel = c;
	drawbars();
}

/* there are some broken focus acquiring clients needing extra handling */
void focusin(XEvent *e)
{
	XFocusChangeEvent *ev = &e->xfocus;

	if (selmon->sel && ev->window != selmon->sel->win) setfocus(selmon->sel);
}

void focusmon(const Arg *arg)
{
	Monitor *m;

	if (!mons->next) return;
	if ((m = dirtomon(arg->i)) == selmon) return;
	unfocus(selmon->sel, 0);
	selmon = m;
	focus(NULL);

	// Thank you to u/bakkeby on r/dwm for the solution, which was derived from the cursor warp patch
	if (selmon->sel)
		XWarpPointer(dpy, None, selmon->sel->win, 0, 0, 0, 0, selmon->sel->w / 2, selmon->sel->h / 2);
	else
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->wx + selmon->ww / 2, selmon->wy + selmon->wh / 2);
}

void focusstack(const Arg *arg)
{
	Client *c = NULL, *i;

	if (!selmon->sel || (selmon->sel->isfullscreen && selmon->sel->fakefullscreen != 1)) return;
	if (arg->i > 0) {
		for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
		if (!c)
			for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
	} else {
		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (ISVISIBLE(i)) c = i;
		if (!c)
			for (; i; i = i->next)
				if (ISVISIBLE(i)) c = i;
	}
	if (c) {
		focus(c);
		restack(selmon);
		XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
	}
}

void freeicon(Client *c)
{
	if (c->icon) {
		XRenderFreePicture(dpy, c->icon);
		c->icon = None;
	}
}

void fullscreencheck(Client *c)
{
	if (!c) return;
	if (c->fakefullscreen == 0 && c->isfullscreen)
		togglefakefullscreen(0);
	else if (c->fakefullscreen == 2)
		togglefullscreen(0);
}

void gaplessgrid(Monitor *m)
{
	unsigned int i, n;
	int x, y, cols, rows, ch, cw, cn, rn, rrest, crest; // counters
	int oh, ov, ih, iv;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	/* grid dimensions */
	for (cols = 0; cols <= n / 2; cols++)
		if (cols * cols >= n) break;
	if (n == 5) /* set layout against the general calculation: not 1:2:2, but 2:3 */
		cols = 2;
	rows = n / cols;
	cn = rn = 0; // reset column no, row no, client count

	ch = (m->wh - 2 * oh - ih * (rows - 1)) / rows;
	cw = (m->ww - 2 * ov - iv * (cols - 1)) / cols;
	rrest = (m->wh - 2 * oh - ih * (rows - 1)) - ch * rows;
	crest = (m->ww - 2 * ov - iv * (cols - 1)) - cw * cols;
	x = m->wx + ov;
	y = m->wy + oh;

	for (i = 0, c = nexttiled(m->clients); c; i++, c = nexttiled(c->next)) {
		if (i / rows + 1 > cols - n % cols) {
			rows = n / cols + 1;
			ch = (m->wh - 2 * oh - ih * (rows - 1)) / rows;
			rrest = (m->wh - 2 * oh - ih * (rows - 1)) - ch * rows;
		}
		resize(c, x, y + rn * (ch + ih) + MIN(rn, rrest), cw + (cn < crest ? 1 : 0) - 2 * c->bw,
		       ch + (rn < rrest ? 1 : 0) - 2 * c->bw, 0);
		rn++;
		if (rn >= rows) {
			rn = 0;
			x += cw + ih + (cn < crest ? 1 : 0);
			cn++;
		}
	}
}

Atom getatomprop(Client *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	/* FIXME getatomprop should return the number of items and a pointer to
	 * the stored data instead of this workaround */
	Atom req = XA_ATOM;
	if (prop == xatom[XembedInfo]) req = xatom[XembedInfo];

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, req, &da, &di, &dl, &dl, &p) == Success &&
	    p) {
		atom = *(Atom *)p;
		if (da == xatom[XembedInfo] && dl == 2) atom = ((Atom *)p)[1];
		XFree(p);
	}
	return atom;
}

void getfacts(Monitor *m, int msize, int ssize, float *mf, float *sf, int *mr, int *sr)
{
	unsigned int n;
	float mfacts, sfacts;
	int mtotal = 0, stotal = 0;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	mfacts = MIN(n, m->nmaster);
	sfacts = n - m->nmaster;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++)
		if (n < m->nmaster)
			mtotal += msize / mfacts;
		else
			stotal += ssize / sfacts;

	*mf = mfacts; // total factor of master area
	*sf = sfacts; // total factor of stack area
	*mr = msize - mtotal; // the remainder (rest) of pixels after an even master split
	*sr = ssize - stotal; // the remainder (rest) of pixels after an even stack split
}

void getgaps(Monitor *m, int *oh, int *ov, int *ih, int *iv, unsigned int *nc)
{
	unsigned int n, oe, ie;
	oe = ie = selmon->pertag->enablegaps[selmon->pertag->curtag];
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (smartgaps && n == 1) {
		oe = 0; // outer gaps disabled when only one client
	}

	*oh = m->gappoh * oe; // outer horizontal gap
	*ov = m->gappov * oe; // outer vertical gap
	*ih = m->gappih * ie; // inner horizontal gap
	*iv = m->gappiv * ie; // inner vertical gap
	*nc = n; // number of clients
}

Picture geticonprop(Window win, unsigned int *picw, unsigned int *pich)
{
	int format;
	unsigned long n, extra, *p = NULL;
	Atom real;

	if (XGetWindowProperty(dpy, win, netatom[NetWMIcon], 0L, LONG_MAX, False, AnyPropertyType, &real, &format, &n,
			       &extra, (unsigned char **)&p) != Success)
		return None;
	if (n == 0 || format != 32) {
		XFree(p);
		return None;
	}

	unsigned long *bstp = NULL;
	uint32_t w, h, sz;
	{
		unsigned long *i;
		const unsigned long *end = p + n;
		uint32_t bstd = UINT32_MAX, d, m;
		for (i = p; i < end - 1; i += sz) {
			if ((w = *i++) >= 16384 || (h = *i++) >= 16384) {
				XFree(p);
				return None;
			}
			if ((sz = w * h) > end - i) break;
			if ((m = w > h ? w : h) >= ICONSIZE && (d = m - ICONSIZE) < bstd) {
				bstd = d;
				bstp = i;
			}
		}
		if (!bstp) {
			for (i = p; i < end - 1; i += sz) {
				if ((w = *i++) >= 16384 || (h = *i++) >= 16384) {
					XFree(p);
					return None;
				}
				if ((sz = w * h) > end - i) break;
				if ((d = ICONSIZE - (w > h ? w : h)) < bstd) {
					bstd = d;
					bstp = i;
				}
			}
		}
		if (!bstp) {
			XFree(p);
			return None;
		}
	}

	if ((w = *(bstp - 2)) == 0 || (h = *(bstp - 1)) == 0) {
		XFree(p);
		return None;
	}

	uint32_t icw, ich;
	if (w <= h) {
		ich = ICONSIZE;
		icw = w * ICONSIZE / h;
		if (icw == 0) icw = 1;
	} else {
		icw = ICONSIZE;
		ich = h * ICONSIZE / w;
		if (ich == 0) ich = 1;
	}
	*picw = icw;
	*pich = ich;

	uint32_t i, *bstp32 = (uint32_t *)bstp;
	for (sz = w * h, i = 0; i < sz; ++i) bstp32[i] = prealpha(bstp[i]);

	Picture ret = drw_picture_create_resized(drw, (char *)bstp, w, h, icw, ich);
	XFree(p);

	return ret;
}

int getrootptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long getstate(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState], &real, &format, &n, &extra,
			       (unsigned char **)&p) != Success)
		return -1;
	if (n != 0) result = *p;
	XFree(p);
	return result;
}

unsigned int getsystraywidth(void)
{
	unsigned int w = 0;
	Client *i;
	if (showsystray)
		for (i = systray->icons; i; w += i->w + systrayspacing, i = i->next);
	return w ? w + systrayspacing : 1;
}

int gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0) return 0;
	text[0] = '\0';
	if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems) return 0;
	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

void grabbuttons(Client *c, int focused)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask | LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!focused)
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK, GrabModeSync, GrabModeSync,
				    None, None);
		for (i = 0; i < LENGTH(buttons); i++)
			if (buttons[i].click == ClkClientWin)
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabButton(dpy, buttons[i].button, buttons[i].mask | modifiers[j], c->win,
						    False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
	}
}

void grabkeys(void)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask | LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for (i = 0; i < LENGTH(keys); i++)
			if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(dpy, code, keys[i].mod | modifiers[j], root, True, GrabModeAsync,
						 GrabModeAsync);
	}
}

void grid(Monitor *m)
{
	unsigned int i, n;
	int cx, cy, cw, ch, cc, cr, chrest, cwrest, cols, rows;
	int oh, ov, ih, iv;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);

	/* grid dimensions */
	for (rows = 0; rows <= n / 2; rows++)
		if (rows * rows >= n) break;
	cols = (rows && (rows - 1) * rows >= n) ? rows - 1 : rows;

	/* window geoms (cell height/width) */
	ch = (m->wh - 2 * oh - ih * (rows - 1)) / (rows ? rows : 1);
	cw = (m->ww - 2 * ov - iv * (cols - 1)) / (cols ? cols : 1);
	chrest = (m->wh - 2 * oh - ih * (rows - 1)) - ch * rows;
	cwrest = (m->ww - 2 * ov - iv * (cols - 1)) - cw * cols;
	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		cc = i / rows;
		cr = i % rows;
		cx = m->wx + ov + cc * (cw + iv) + MIN(cc, cwrest);
		cy = m->wy + oh + cr * (ch + ih) + MIN(cr, chrest);
		resize(c, cx, cy, cw + (cc < cwrest ? 1 : 0) - 2 * c->bw, ch + (cr < chrest ? 1 : 0) - 2 * c->bw,
		       False);
	}
}

int handlexevent(struct epoll_event *ev)
{
	if (ev->events & EPOLLIN) {
		XEvent ev;
		while (running && XPending(dpy)) {
			XNextEvent(dpy, &ev);
			if (handler[ev.type]) {
				handler[ev.type](&ev); /* call handler */
				ipc_send_events(mons, &lastselmon, selmon);
			}
		}
	} else if (ev->events & EPOLLHUP) {
		return -1;
	}

	return 0;
}

void horizgrid(Monitor *m)
{
	Client *c;
	unsigned int n, i;
	int oh, ov, ih, iv;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	int ntop, nbottom = 1;
	float mfacts, sfacts;
	int mrest, srest;

	/* Count windows */
	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	if (n <= 2)
		ntop = n;
	else {
		ntop = n / 2;
		nbottom = n - ntop;
	}
	sx = mx = m->wx + ov;
	sy = my = m->wy + oh;
	sh = mh = m->wh - 2 * oh;
	sw = mw = m->ww - 2 * ov;

	if (n > ntop) {
		sh = (mh - ih) / 2;
		mh = mh - ih - sh;
		sy = my + mh + ih;
		mw = m->ww - 2 * ov - iv * (ntop - 1);
		sw = m->ww - 2 * ov - iv * (nbottom - 1);
	}

	mfacts = ntop;
	sfacts = nbottom;
	mrest = mw - (mw / ntop) * ntop;
	srest = sw - (sw / nbottom) * nbottom;

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < ntop) {
			resize(c, mx, my, (mw / mfacts) + (i < mrest ? 1 : 0) - (2 * c->bw), mh - (2 * c->bw), 0);
			mx += WIDTH(c) + iv;
		} else {
			resize(c, sx, sy, (sw / sfacts) + ((i - ntop) < srest ? 1 : 0) - (2 * c->bw), sh - (2 * c->bw),
			       0);
			sx += WIDTH(c) + iv;
		}
}

void incnmaster(const Arg *arg)
{
	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

void incrgaps(const Arg *arg)
{
	setgaps(selmon->gappoh + arg->i, selmon->gappov + arg->i, selmon->gappih + arg->i, selmon->gappiv + arg->i);
}

void incrigaps(const Arg *arg)
{
	setgaps(selmon->gappoh, selmon->gappov, selmon->gappih + arg->i, selmon->gappiv + arg->i);
}

void incrihgaps(const Arg *arg)
{
	setgaps(selmon->gappoh, selmon->gappov, selmon->gappih + arg->i, selmon->gappiv);
}

void incrivgaps(const Arg *arg)
{
	setgaps(selmon->gappoh, selmon->gappov, selmon->gappih, selmon->gappiv + arg->i);
}

void incrogaps(const Arg *arg)
{
	setgaps(selmon->gappoh + arg->i, selmon->gappov + arg->i, selmon->gappih, selmon->gappiv);
}

void incrohgaps(const Arg *arg)
{
	setgaps(selmon->gappoh + arg->i, selmon->gappov, selmon->gappih, selmon->gappiv);
}

void incrovgaps(const Arg *arg)
{
	setgaps(selmon->gappoh, selmon->gappov + arg->i, selmon->gappih, selmon->gappiv);
}

#ifdef XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
	while (n--)
		if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org &&
		    unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	return 1;
}
#endif /* XINERAMA */

void keypress(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func)
			keys[i].func(&(keys[i].arg));
}

void killclient(const Arg *arg)
{
	if (!selmon->sel) return;
	if (!sendevent(selmon->sel->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0)) {
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, selmon->sel->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

void layoutscroll(const Arg *arg)
{
	if (!arg || !arg->i) return;
	int switchto = selmon->ltcur + arg->i;
	int l = LENGTH(layouts);

	if (switchto == l)
		switchto = 0;
	else if (switchto < 0)
		switchto = l - 1;

	selmon->ltcur = switchto;
	Arg arg2 = { .v = &layouts[switchto] };
	setlayout(&arg2);
}

void losefullscreen(Client *next)
{
	Client *sel = selmon->sel;
	if (!sel || !next) return;
	if (sel->isfullscreen && sel->fakefullscreen != 1 && ISVISIBLE(sel) && sel->mon == next->mon &&
	    !next->isfloating)
		setfullscreen(sel, 0);
}

void manage(Window w, XWindowAttributes *wa)
{
	Client *c, *t = NULL;
	Window trans = None;
	XWindowChanges wc;

	c = ecalloc(1, sizeof(Client));
	c->win = w;

	if (getatomprop(c, netatom[NetWMWindowType]) == netatom[NetWMWindowTypeDesktop]) {
		XMapWindow(dpy, c->win);
		XLowerWindow(dpy, c->win);
		free(c);
		return;
	}

	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;

	updateicon(c);
	updatetitle(c);
	updatesizehints(c);
	updatewmhints(c);
	if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
		applyrules(c);
	}

	if (unmanaged) {
		XMapWindow(dpy, c->win);
		if (unmanaged == 1)
			XRaiseWindow(dpy, c->win);
		else if (unmanaged == 2)
			XLowerWindow(dpy, c->win);

		updatewmhints(c);
		if (!c->neverfocus) XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		sendevent(c->win, wmatom[WMTakeFocus], StructureNotifyMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);

		free(c);
		unmanaged = 0;
		return;
	}

	updatewindowtype(c);

	{
		int format;
		unsigned long *data, n, extra;
		Monitor *m;
		Atom atom;
		if (XGetWindowProperty(dpy, c->win, netatom[NetClientInfo], 0L, 2L, False, XA_CARDINAL, &atom, &format,
				       &n, &extra, (unsigned char **)&data) == Success &&
		    n == 2) {
			c->tags = *data;
			for (m = mons; m; m = m->next) {
				if (m->num == *(data + 1)) {
					c->mon = m;
					break;
				}
			}
		}
		if (n > 0) XFree(data);
	}

	setclienttagprop(c);

	if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww) {
		c->x = c->mon->wx + c->mon->ww - WIDTH(c);
	}
	if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh) {
		c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
	}

	selmon->tagset[selmon->seltags] &= ~scratchtag;
	if (!strcmp(c->name, scratchpadname)) {
		c->mon->tagset[c->mon->seltags] |= c->tags = scratchtag;
	}

	if (!c->isfloating) c->isfloating = c->oldstate = trans != None || c->isfixed;

	if (c->isfloating) {
		c->bw = fborderpx;
		XRaiseWindow(dpy, c->win);
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColFloat].pixel);
	} else {
		c->bw = borderpx;
		XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
	}

	wc.border_width = c->bw;
	c->x = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
	c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	configure(c); /* propagates border_width, if size doesn't change */
	XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	grabbuttons(c, 0);
	attach(c);
	attachstack(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win),
			1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);
	if (c->mon == selmon) {
		fullscreencheck(selmon->sel);
		unfocus(selmon->sel, 0);
	}
	c->mon->sel = c;
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	if (c && c->mon == selmon) XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
	focus(NULL);
}

void mappingnotify(XEvent *e)
{
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard) grabkeys();
}

void maprequest(XEvent *e)
{
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

	Client *i;
	if ((i = wintosystrayicon(ev->window))) {
		sendevent(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0,
			  systray->win, XEMBED_EMBEDDED_VERSION);
		resizebarwin(selmon);
		updatesystray();
	}

	if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect) {
		return;
	}

	if (!wintoclient(ev->window)) manage(ev->window, &wa);
}

void monocle(Monitor *m)
{
	unsigned int n;
	int oh, ov, ih, iv;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);

	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx + ov, m->wy + oh, m->ww - 2 * c->bw - 2 * ov, m->wh - 2 * c->bw - 2 * oh, 0);
}

void motionnotify(XEvent *e)
{
	static Monitor *mon = NULL;
	Monitor *m;
	XMotionEvent *ev = &e->xmotion;
	if (ev->window != root) return;
	if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	mon = m;
}

void movemouse(const Arg *arg)
{
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel)) return;
	if (c->isfullscreen && c->fakefullscreen != 1) /* no support moving fullscreen windows by mouse */
		return;
	if (c->isfloating != 1) // Dont move windows that aren't floating
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurMove]->cursor,
			 CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y)) return;
	do {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60)) continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if (abs(selmon->wx - nx) < snap)
				nx = selmon->wx;
			else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
				nx = selmon->wx + selmon->ww - WIDTH(c);
			if (abs(selmon->wy - ny) < snap)
				ny = selmon->wy;
			else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
				ny = selmon->wy + selmon->wh - HEIGHT(c);
			if (!c->isfloating && selmon->lt[selmon->sellt]->arrange &&
			    (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
				togglefloating(NULL);
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating) resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

Client *nexttiled(Client *c)
{
	for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void nrowgrid(Monitor *m)
{
	unsigned int n;
	int ri = 0, ci = 0; /* counters */
	int oh, ov, ih, iv; /* vanitygap settings */
	unsigned int cx, cy, cw, ch; /* client geometry */
	unsigned int uw = 0, uh = 0, uc = 0; /* utilization trackers */
	unsigned int cols, rows = m->nmaster + 1;
	Client *c;

	/* count clients */
	getgaps(m, &oh, &ov, &ih, &iv, &n);

	/* nothing to do here */
	if (n == 0) return;

	/* force 2 clients to always split vertically */
	if (FORCE_VSPLIT && n == 2) rows = 1;

	/* never allow empty rows */
	if (n < rows) rows = n;

	/* define first row */
	cols = n / rows;
	uc = cols;
	cy = m->wy + oh;
	ch = (m->wh - 2 * oh - ih * (rows - 1)) / rows;
	uh = ch;

	for (c = nexttiled(m->clients); c; c = nexttiled(c->next), ci++) {
		if (ci == cols) {
			uw = 0;
			ci = 0;
			ri++;

			/* next row */
			cols = (n - uc) / (rows - ri);
			uc += cols;
			cy = m->wy + oh + uh + ih;
			uh += ch + ih;
		}

		cx = m->wx + ov + uw;
		cw = (m->ww - 2 * ov - uw) / (cols - ci);
		uw += cw + iv;

		resize(c, cx, cy, cw - (2 * c->bw), ch - (2 * c->bw), 0);
	}
}

void pop(Client *c)
{
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

uint32_t prealpha(uint32_t p)
{
	uint8_t a = p >> 24u;
	uint32_t rb = (a * (p & 0xFF00FFu)) >> 8u;
	uint32_t g = (a * (p & 0x00FF00u)) >> 8u;
	return (rb & 0xFF00FFu) | (g & 0x00FF00u) | (a << 24u);
}

void propertynotify(XEvent *e)
{
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if ((c = wintosystrayicon(ev->window))) {
		if (ev->atom == XA_WM_NORMAL_HINTS) {
			updatesizehints(c);
			updatesystrayicongeom(c, c->w, c->h);
		} else
			updatesystrayiconstate(c, ev);
		resizebarwin(selmon);
		updatesystray();
	}
	if ((ev->window == root) && (ev->atom == XA_WM_NAME))
		updatestatus();
	else if (ev->state == PropertyDelete)
		return; /* ignore */
	else if ((c = wintoclient(ev->window))) {
		switch (ev->atom) {
		default:
			break;
		case XA_WM_TRANSIENT_FOR:
			if (!c->ignoretransient && !c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
			    (c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			c->hintsvalid = 0;
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == c->mon->sel && showtitle) drawbar(c->mon);
		} else if (ev->atom == netatom[NetWMIcon]) {
			updateicon(c);
			if (c == c->mon->sel) drawbar(c->mon);
		}
		if (ev->atom == netatom[NetWMWindowType]) updatewindowtype(c);
	}
}

void quit(const Arg *arg)
{
	if (arg->i) restart = 1;
	running = 0;
}

Monitor *recttomon(int x, int y, int w, int h)
{
	Monitor *m, *r = selmon;
	int a, area = 0;

	for (m = mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

void removesystrayicon(Client *i)
{
	Client **ii;

	XReparentWindow(
		dpy, i->win, root, 0,
		0); // https://www.reddit.com/r/suckless/comments/vdpq14/comment/icpsnhe/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
	if (!showsystray || !i) return;
	for (ii = &systray->icons; *ii && *ii != i; ii = &(*ii)->next);
	if (ii) *ii = i->next;
	free(i);
}

void resize(Client *c, int x, int y, int w, int h, int interact)
{
	if (applysizehints(c, &x, &y, &w, &h, interact)) resizeclient(c, x, y, w, h);
}

void resizebarwin(Monitor *m)
{
	unsigned int w = m->ww - 2 * sp;
	if (showsystray && m == systraytomon(m) && !systrayonleft) w -= getsystraywidth();
	XMoveResizeWindow(dpy, m->barwin, m->wx + sp, m->by + vp, w, bh);
}

void resizeclient(Client *c, int x, int y, int w, int h)
{
	XWindowChanges wc;

	c->oldx = c->x;
	c->x = wc.x = x;
	c->oldy = c->y;
	c->y = wc.y = y;
	c->oldw = c->w;
	c->w = wc.width = w;
	c->oldh = c->h;
	c->h = wc.height = h;
	wc.border_width = c->bw;
	XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void resizemouse(const Arg *arg)
{
	int ocx, ocy, nw, nh;
	int ocx2, ocy2, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	int horizcorner = 0, vertcorner = 0;
	int di;
	unsigned int dui;
	Window dummy;
	Time lasttime = 0;

	if (!(c = selmon->sel)) return;
	if (c->isfullscreen && c->fakefullscreen != 1) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	ocx2 = c->x + c->w;
	ocy2 = c->y + c->h;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurResize]->cursor,
			 CurrentTime) != GrabSuccess)
		return;
	if (c->isfloating || NULL == c->mon->lt[c->mon->sellt]->arrange) {
		if (!XQueryPointer(dpy, c->win, &dummy, &dummy, &di, &di, &nx, &ny, &dui)) return;
		horizcorner = nx < c->w / 2;
		vertcorner = ny < c->h / 2;
		XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, horizcorner ? (-c->bw) : (c->w + c->bw - 1),
			     vertcorner ? (-c->bw) : (c->h + c->bw - 1));
	} else {
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->mx + (selmon->ww * selmon->mfact),
			     selmon->my + (selmon->wh / 2));
	}

	do {
		XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60)) continue;
			lasttime = ev.xmotion.time;

			nx = horizcorner ? ev.xmotion.x : c->x;
			ny = vertcorner ? ev.xmotion.y : c->y;
			nw = MAX(horizcorner ? (ocx2 - nx) : (ev.xmotion.x - ocx - 2 * c->bw + 1), 1);
			nh = MAX(vertcorner ? (ocy2 - ny) : (ev.xmotion.y - ocy - 2 * c->bw + 1), 1);

			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating) resize(c, nx, ny, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);

	if (c->isfloating || NULL == c->mon->lt[c->mon->sellt]->arrange) {
		XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, horizcorner ? (-c->bw) : (c->w + c->bw - 1),
			     vertcorner ? (-c->bw) : (c->h + c->bw - 1));
	} else {
		selmon->mfact = (double)(ev.xmotion.x_root - selmon->mx) / (double)selmon->ww;
		arrange(selmon);
		XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->mx + (selmon->ww * selmon->mfact),
			     selmon->my + (selmon->wh / 2));
		selmon->pertag->mfacts[selmon->pertag->curtag] = selmon->mfact;
	}

	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void resizerequest(XEvent *e)
{
	XResizeRequestEvent *ev = &e->xresizerequest;
	Client *i;

	if ((i = wintosystrayicon(ev->window))) {
		updatesystrayicongeom(i, ev->width, ev->height);
		resizebarwin(selmon);
		updatesystray();
	}
}

void restack(Monitor *m)
{
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar(m);
	if (!m->sel) return;
	if (m->sel->isfloating || !m->lt[m->sellt]->arrange) XRaiseWindow(dpy, m->sel->win);
	if (m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for (c = m->stack; c; c = c->snext)
			if (!c->isfloating && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void rotatestack(const Arg *arg)
{
	Client *c = NULL, *f;

	if (!selmon->sel) return;
	f = selmon->sel;
	if (arg->i > 0) {
		for (c = nexttiled(selmon->clients); c && nexttiled(c->next); c = nexttiled(c->next));
		if (c) {
			detach(c);
			attach(c);
			detachstack(c);
			attachstack(c);
		}
	} else {
		if ((c = nexttiled(selmon->clients))) {
			detach(c);
			enqueue(c);
			detachstack(c);
			enqueuestack(c);
		}
	}
	if (c) {
		arrange(selmon);
		//unfocus(f, 1);
		focus(f);
		restack(selmon);
	}
}

void run(void)
{
	int event_count = 0;
	const int MAX_EVENTS = 10;
	struct epoll_event events[MAX_EVENTS];

	XSync(dpy, False);

	/* main event loop */
	while (running) {
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		for (int i = 0; i < event_count; i++) {
			int event_fd = events[i].data.fd;
			DEBUG("Got event from fd %d\n", event_fd);

			if (event_fd == dpy_fd) {
				// -1 means EPOLLHUP
				if (handlexevent(events + i) == -1) return;
			} else if (event_fd == ipc_get_sock_fd()) {
				ipc_handle_socket_epoll_event(events + i);
			} else if (ipc_is_client_registered(event_fd)) {
				if (ipc_handle_client_epoll_event(events + i, mons, &lastselmon, selmon, tags,
								  LENGTH(tags), layouts, LENGTH(layouts)) < 0) {
					fprintf(stderr, "Error handling IPC event on fd %d\n", event_fd);
				}
			} else {
				fprintf(stderr, "Got event from unknown fd %d, ptr %p, u32 %d, u64 %lu", event_fd,
					events[i].data.ptr, events[i].data.u32, events[i].data.u64);
				fprintf(stderr, " with events %d\n", events[i].events);
				return;
			}
		}
	}
}

void scan(void)
{
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa) || wa.override_redirect ||
			    XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState) manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa)) continue;
			if (XGetTransientForHint(dpy, wins[i], &d1) &&
			    (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		if (wins) XFree(wins);
	}
}

int sendevent(Window w, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4)
{
	int n;
	Atom *protocols, mt;
	int exists = 0;
	XEvent ev;

	if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
		mt = wmatom[WMProtocols];
		if (XGetWMProtocols(dpy, w, &protocols, &n)) {
			while (!exists && n--) exists = protocols[n] == proto;
			XFree(protocols);
		}
	} else {
		exists = True;
		mt = proto;
	}
	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = w;
		ev.xclient.message_type = mt;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = d0;
		ev.xclient.data.l[1] = d1;
		ev.xclient.data.l[2] = d2;
		ev.xclient.data.l[3] = d3;
		ev.xclient.data.l[4] = d4;
		XSendEvent(dpy, w, False, mask, &ev);
	}
	return exists;
}

void sendmon(Client *c, Monitor *m)
{
	if (c->mon == m) return;
	unfocus(c, 1);
	detach(c);
	detachstack(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	c->x = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
	c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;
	if (!strcmp(c->name, scratchpadname)) {
		c->mon->tagset[c->mon->seltags] |= c->tags = scratchtag;
	}
	attach(c);
	attachstack(c);
	setclienttagprop(c);
	focus(NULL);
	arrange(NULL);
}

void setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

void setclienttagprop(Client *c)
{
	long data[] = { (long)c->tags, (long)c->mon->num };
	XChangeProperty(dpy, c->win, netatom[NetClientInfo], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
			2);
}

void setcurrentdesktop(void)
{
	long data[] = { 0 };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
			1);
}

void setdesktopnames(void)
{
	XTextProperty text;

// Added because im lazy and dont want to have to write IPC to solve a simple warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	Xutf8TextListToTextProperty(dpy, tags, TAGSLENGTH, XUTF8StringStyle, &text);
#pragma GCC diagnostic pop

	XSetTextProperty(dpy, root, &text, netatom[NetDesktopNames]);
}

void setfocus(Client *c)
{
	if (!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace,
				(unsigned char *)&(c->win), 1);
	}
	sendevent(c->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void setfullscreen(Client *c, int fullscreen)
{
	XEvent ev;
	int savestate = 0, restorestate = 0, restorefakefullscreen = 0;

	if ((c->fakefullscreen == 0 && fullscreen && !c->isfullscreen) // normal fullscreen
	    || (c->fakefullscreen == 2 && fullscreen)) // fake fullscreen --> actual fullscreen
		savestate = 1; // go actual fullscreen
	else if ((c->fakefullscreen == 0 && !fullscreen && c->isfullscreen) // normal fullscreen exit
		 || (c->fakefullscreen >= 2 && !fullscreen)) // fullscreen exit --> fake fullscreen
		restorestate = 1; // go back into tiled

	/* If leaving fullscreen and the window was previously fake fullscreen (2), then restore
     * that while staying in fullscreen. The exception to this is if we are in said state, but
     * the client itself disables fullscreen (3) then we let the client go out of fullscreen
     * while keeping fake fullscreen enabled (as otherwise there will be a mismatch between the
     * client and the window manager's perception of the client's fullscreen state). */
	if (c->fakefullscreen == 2 && !fullscreen && c->isfullscreen) {
		restorefakefullscreen = 1;
		c->isfullscreen = 1;
		fullscreen = 1;
	}

	if (fullscreen != c->isfullscreen) { // only send property change if necessary
		if (fullscreen)
			XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace,
					(unsigned char *)&netatom[NetWMFullscreen], 1);
		else
			XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace,
					(unsigned char *)0, 0);
	}

	c->isfullscreen = fullscreen;

	/* Some clients, e.g. firefox, will send a client message informing the window manager
     * that it is going into fullscreen after receiving the above signal. This has the side
     * effect of this function (setfullscreen) sometimes being called twice when toggling
     * fullscreen on and off via the window manager as opposed to the application itself.
     * To protect against obscure issues where the client settings are stored or restored
     * when they are not supposed to we add an additional bit-lock on the old state so that
     * settings can only be stored and restored in that precise order. */
	if (savestate && !(c->oldstate & (1 << 1))) {
		c->oldbw = c->bw;
		c->oldstate = c->isfloating | (1 << 1);
        if (c->isfloating) {
            c->oldfx = c->x;
		    c->oldfy = c->y;
		    c->oldfw = c->w;
		    c->oldfh = c->h;
        }
		c->bw = 0;
		c->isfloating = 1;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (restorestate && (c->oldstate & (1 << 1))) {
		c->bw = c->oldbw;
		c->isfloating = c->oldstate = c->oldstate & 1;
		if (restorefakefullscreen || c->fakefullscreen == 3) c->fakefullscreen = 1;
		/* The client may have been moved to another monitor whilst in fullscreen which if tiled
         * we address by doing a full arrange of tiled clients. If the client is floating then the
         * height and width may be larger than the monitor's window area, so we cap that by
         * ensuring max / min values. */
		if (c->isfloating) {
			int borderdiff = (fborderpx - borderpx) * 2;
			resize(c, c->oldfx, c->oldfy, c->oldfw - borderdiff, c->oldfh - borderdiff, 0);
			restack(c->mon);
		} else
			arrange(c->mon);
	} else
		resizeclient(c, c->x, c->y, c->w, c->h);

	/* Exception: if the client was in actual fullscreen and we exit out to fake fullscreen
     * mode, then the focus would sometimes drift to whichever window is under the mouse cursor
     * at the time. To avoid this we ask X for all EnterNotify events and just ignore them.
     */
	if (!c->isfullscreen)
		while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void setgaps(int oh, int ov, int ih, int iv)
{
	if (oh < 0) oh = 0;
	if (ov < 0) ov = 0;
	if (ih < 0) ih = 0;
	if (iv < 0) iv = 0;

	selmon->gappoh = oh;
	selmon->gappov = ov;
	selmon->gappih = ih;
	selmon->gappiv = iv;
	arrange(selmon);
}

void setlayout(const Arg *arg)
{
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] =
			(Layout *)arg->v;

// Added because im lazy and dont want deal with the error, causes no known issues
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
#pragma GCC diagnostic pop

	if (selmon->sel)
		arrange(selmon);
	else
		drawbar(selmon);
}

void setlayoutsafe(const Arg *arg)
{
	const Layout *ltptr = (Layout *)arg->v;
	if (ltptr == 0) setlayout(arg);
	for (int i = 0; i < LENGTH(layouts); i++) {
		if (ltptr == &layouts[i]) setlayout(arg);
	}
}

/* arg > 1.0 will set mfact absolutely */
void setmfact(const Arg *arg)
{
	float f;

	if (!arg || !selmon->lt[selmon->sellt]->arrange) return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95) return;
	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
	arrange(selmon);
}

void setnumdesktops(void)
{
	long data[] = { TAGSLENGTH };
	XChangeProperty(dpy, root, netatom[NetNumberOfDesktops], XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *)data, 1);
}

void setsticky(Client *c, int sticky)
{

	if (sticky && !c->issticky) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace,
				(unsigned char *)&netatom[NetWMSticky], 1);
		c->issticky = 1;
	} else if (!sticky && c->issticky) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)0, 0);
		c->issticky = 0;
		arrange(c->mon);
	}
}

void setup(void)
{
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;
	struct sigaction sa;

	/* restartsig */
	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);

	/* do not transform children into zombies when they terminate */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	/* clean up any zombies (inherited from .xinitrc etc) immediately */
	while (waitpid(-1, NULL, WNOHANG) > 0) {
	};

	/* init screen */
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);
	drw = drw_create(dpy, screen, root, sw, sh);
	if (!drw_fontset_create(drw, fonts, LENGTH(fonts))) die("no fonts could be loaded.");
	lrpad = drw->fonts->h + horizpadbar;
	bh = drw->fonts->h + vertpadbar;
	sp = sidepad;
	vp = (topbar == 1) ? vertpad : -vertpad;
	updategeom();

	/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetSystemTray] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_S0", False);
	netatom[NetSystemTrayOP] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
	netatom[NetSystemTrayOrientation] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False);
	netatom[NetSystemTrayOrientationHorz] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMIcon] = XInternAtom(dpy, "_NET_WM_ICON", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMSticky] = XInternAtom(dpy, "_NET_WM_STATE_STICKY", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetWMWindowTypeDesktop] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	netatom[NetDesktopViewport] = XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", False);
	netatom[NetNumberOfDesktops] = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	netatom[NetCurrentDesktop] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
	netatom[NetDesktopNames] = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	xatom[Manager] = XInternAtom(dpy, "MANAGER", False);
	xatom[Xembed] = XInternAtom(dpy, "_XEMBED", False);
	xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);
	netatom[NetClientInfo] = XInternAtom(dpy, "_NET_CLIENT_INFO", False);

	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);

	/* init appearance */
	scheme = ecalloc(LENGTH(colors), sizeof(Clr *));
	for (i = 0; i < LENGTH(colors); i++) {
		scheme[i] = drw_scm_create(drw, colors[i], 4);
	}

	/* init system tray */
	updatesystray();

	/* init bars */
	updatebars();
	updatestatus();

	/* supporting window for NetWMCheck */
	wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace,
			(unsigned char *)&wmcheckwin, 1);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8, PropModeReplace, (unsigned char *)"jdwm",
			8);
	XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin,
			1);

	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *)netatom,
			NetLast);
	setnumdesktops();
	setcurrentdesktop();
	setdesktopnames();
	setviewport();
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	XDeleteProperty(dpy, root, netatom[NetClientInfo]);

	/* select events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask |
			EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	grabkeys();
	focus(NULL);
	setupepoll();
}

void setupepoll(void)
{
	epoll_fd = epoll_create1(0);
	dpy_fd = ConnectionNumber(dpy);
	struct epoll_event dpy_event;

	// Initialize struct to 0
	memset(&dpy_event, 0, sizeof(dpy_event));

	DEBUG("Display socket is fd %d\n", dpy_fd);

	if (epoll_fd == -1) {
		fputs("Failed to create epoll file descriptor", stderr);
	}

	dpy_event.events = EPOLLIN;
	dpy_event.data.fd = dpy_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, dpy_fd, &dpy_event)) {
		fputs("Failed to add display file descriptor to epoll", stderr);
		close(epoll_fd);
		exit(1);
	}

	if (ipc_init(ipcsockpath, epoll_fd, ipccommands, LENGTH(ipccommands)) < 0) {
		fputs("Failed to initialize IPC\n", stderr);
	}
}

void seturgent(Client *c, int urg)
{
	XWMHints *wmh;

	c->isurgent = urg;
	if (!(wmh = XGetWMHints(dpy, c->win))) return;
	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void setviewport(void)
{
	long data[] = { 0, 0 };
	XChangeProperty(dpy, root, netatom[NetDesktopViewport], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
			2);
}

/* move the current active window to the next/prev tag and view it. More like following the window */
void shiftboth(const Arg *arg)
{
	Arg shifted;
	shifted.ui = selmon->tagset[selmon->seltags];

	if (arg->i > 0) /* left circular shift */
		shifted.ui = ((shifted.ui << arg->i) | (shifted.ui >> (LENGTH(tags) - arg->i)));
	else /* right circular shift */
		shifted.ui = ((shifted.ui >> (-arg->i) | shifted.ui << (LENGTH(tags) + arg->i)));
	tag(&shifted);
	view(&shifted);
}

/* swaps "tags" (all the clients) with the next/prev tag. */
void shiftswaptags(const Arg *arg)
{
	Arg shifted;
	shifted.ui = selmon->tagset[selmon->seltags];

	if (arg->i > 0) /* left circular shift */
		shifted.ui = ((shifted.ui << arg->i) | (shifted.ui >> (LENGTH(tags) - arg->i)));
	else /* right circular shift */
		shifted.ui = ((shifted.ui >> (-arg->i) | shifted.ui << (LENGTH(tags) + arg->i)));
	swaptags(&shifted);
	// uncomment if you also want to "go" (view) the tag where the the clients are going
	//view(&shifted);
}

/* Sends a window to the next/prev tag */
void shifttag(const Arg *arg)
{
	Arg shifted;
	shifted.ui = selmon->tagset[selmon->seltags];

	if (arg->i > 0) /* left circular shift */
		shifted.ui = ((shifted.ui << arg->i) | (shifted.ui >> (LENGTH(tags) - arg->i)));
	else /* right circular shift */
		shifted.ui = (shifted.ui >> (-arg->i) | shifted.ui << (LENGTH(tags) + arg->i));
	tag(&shifted);
}

/* Sends a window to the next/prev tag that has a client, else it moves it to the next/prev one. */
void shifttagclients(const Arg *arg)
{

	Arg shifted;
	Client *c;
	unsigned int tagmask = 0;
	shifted.ui = selmon->tagset[selmon->seltags];

	for (c = selmon->clients; c; c = c->next)
		if (!(c->tags)) tagmask = tagmask | c->tags;

	if (arg->i > 0) /* left circular shift */
		do {
			shifted.ui = (shifted.ui << arg->i) | (shifted.ui >> (LENGTH(tags) - arg->i));
		} while (tagmask && !(shifted.ui & tagmask));
	else /* right circular shift */
		do {
			shifted.ui = (shifted.ui >> (-arg->i) | shifted.ui << (LENGTH(tags) + arg->i));
		} while (tagmask && !(shifted.ui & tagmask));
	tag(&shifted);
}

/* Navigate to the next/prev tag */
void shiftview(const Arg *arg)
{
	Arg shifted;
	shifted.ui = selmon->tagset[selmon->seltags];

	if (arg->i > 0) /* left circular shift */
		shifted.ui = (shifted.ui << arg->i) | (shifted.ui >> (LENGTH(tags) - arg->i));
	else /* right circular shift */
		shifted.ui = (shifted.ui >> (-arg->i) | shifted.ui << (LENGTH(tags) + arg->i));
	view(&shifted);
}

/*

Function is completely fucked, will fix later

 Navigate to the next/prev tag that has a client, else moves it to the next/prev tag
void shiftviewclients(const Arg *arg)
{
	Arg shifted;
	Client *c;
	unsigned int tagmask = 0;
	shifted.ui = selmon->tagset[selmon->seltags];

	for (c = selmon->clients; c; c = c->next)
		if (!(c->tags)) tagmask = tagmask | c->tags;

	if (arg->i > 0) // left circular shift 
		do {
			shifted.ui = (shifted.ui << arg->i) | (shifted.ui >> (LENGTH(tags) - arg->i));
		} while (tagmask && !(shifted.ui & tagmask));
	else // right circular shift
		do {
			shifted.ui = (shifted.ui >> (-arg->i) | shifted.ui << (LENGTH(tags) + arg->i));
		} while (tagmask && !(shifted.ui & tagmask));
	view(&shifted);
}
*/

void showhide(Client *c)
{
	if (!c) return;
	if (ISVISIBLE(c)) {
		/* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, 0);
		showhide(c->snext);
	} else {
		/* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
}

void sigchld(int unused)
{
	if (signal(SIGCHLD, sigchld) == SIG_ERR) {
		die("can't install SIGCHLD handler:");
	}
}

void sighup(int unused)
{
	Arg a = { .i = 1 };
	quit(&a);
}

void sigterm(int unused)
{
	Arg a = { .i = 0 };
	quit(&a);
}

void spawn(const Arg *arg)
{
	struct sigaction sa;
	if (fork() == 0) {
		if (dpy) close(ConnectionNumber(dpy));
		setsid();
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);
		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("jdwm: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

void spiral(Monitor *m)
{
	fibonacci(m, 0);
}

//helper function for shiftswaptags.
//see: https://github.com/moizifty/DWM-Build/blob/65379c62640788881486401a0d8c79333751b02f/config.h#L48
void swaptags(const Arg *arg)
{
	Client *c;
	unsigned int newtag = arg->ui & TAGMASK;
	unsigned int curtag = selmon->tagset[selmon->seltags];

	if (newtag == curtag || !curtag || (curtag & (curtag - 1))) return;

	for (c = selmon->clients; c != NULL; c = c->next) {
		if ((c->tags & newtag) || (c->tags & curtag)) c->tags ^= curtag ^ newtag;

		if (!c->tags) c->tags = newtag;
	}

	//move to the swaped tag
	//selmon->tagset[selmon->seltags] = newtag;

	focus(NULL);
	arrange(selmon);
}

Monitor *systraytomon(Monitor *m)
{
	Monitor *t;
	int i, n;
	if (!systraypinning) {
		if (!m) return selmon;
		return m == selmon ? m : NULL;
	}
	for (n = 1, t = mons; t && t->next; n++, t = t->next);
	for (i = 1, t = mons; t && t->next && i < systraypinning; i++, t = t->next);
	if (systraypinningfailfirst && n < systraypinning) return mons;
	return t;
}

void tag(const Arg *arg)
{
	Client *c;
	if (selmon->sel && arg->ui & TAGMASK) {
		c = selmon->sel;
		selmon->sel->tags = arg->ui & TAGMASK;
		setclienttagprop(c);
		focus(NULL);
		arrange(selmon);
	}
}

void tagmon(const Arg *arg)
{
	Client *c = selmon->sel;
	if (!c || !mons->next) return;
	if (c->isfullscreen) {
		c->isfullscreen = 0;
		sendmon(c, dirtomon(arg->i));
		c->isfullscreen = 1;
		if (c->fakefullscreen != 1) {
			resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
			XRaiseWindow(dpy, c->win);
		}
	} else
		sendmon(c, dirtomon(arg->i));
}

void tile(Monitor *m)
{
	unsigned int i, n;
	int oh, ov, ih, iv;
	int mx = 0, my = 0, mh = 0, mw = 0;
	int sx = 0, sy = 0, sh = 0, sw = 0;
	float mfacts, sfacts;
	int mrest, srest;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0) return;

	sx = mx = m->wx + ov;
	sy = my = m->wy + oh;
	mh = m->wh - 2 * oh - ih * (MIN(n, m->nmaster) - 1);
	sh = m->wh - 2 * oh - ih * (n - m->nmaster - 1);
	sw = mw = m->ww - 2 * ov;

	if (m->nmaster && n > m->nmaster) {
		sw = (mw - iv) * (1 - m->mfact);
		mw = mw - iv - sw;
		sx = mx + mw + iv;
	}

	getfacts(m, mh, sh, &mfacts, &sfacts, &mrest, &srest);

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			resize(c, mx, my, mw - (2 * c->bw), (mh / mfacts) + (i < mrest ? 1 : 0) - (2 * c->bw), 0);
			my += HEIGHT(c) + ih;
		} else {
			resize(c, sx, sy, sw - (2 * c->bw),
			       (sh / sfacts) + ((i - m->nmaster) < srest ? 1 : 0) - (2 * c->bw), 0);
			sy += HEIGHT(c) + ih;
		}
}

void togglebar(const Arg *arg)
{
	selmon->showbar = selmon->pertag->showbars[selmon->pertag->curtag] = !selmon->showbar;
	updatebarpos(selmon);
	resizebarwin(selmon);
	if (showsystray) {
		XWindowChanges wc;
		if (!selmon->showbar)
			wc.y = -bh;
		else if (selmon->showbar) {
			wc.y = vp;
			if (!selmon->topbar) wc.y = selmon->mh - bh + vp;
		}
		XConfigureWindow(dpy, systray->win, CWY, &wc);
	}
	arrange(selmon);
}

void togglefakefullscreen(const Arg *arg)
{
	Client *c = selmon->sel;
	if (!c) return;

	if (c->fakefullscreen != 1 && c->isfullscreen) { // exit fullscreen --> fake fullscreen
		c->fakefullscreen = 2;
		setfullscreen(c, 0);
	} else if (c->fakefullscreen == 1) {
		setfullscreen(c, 0);
		c->fakefullscreen = 0;
	} else {
		c->fakefullscreen = 1;
		setfullscreen(c, 1);
	}
}

void togglefloating(const Arg *arg)
{
	Client *c = selmon->sel;
	if (!c) return;
	if (c->isfullscreen && c->fakefullscreen != 1) /* no support for fullscreen windows */
		return;
	c->isfloating = !c->isfloating || c->isfixed;
	if (c->isfloating) {
		XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColFloat].pixel);
		c->bw = fborderpx;
		configure(c);
		int borderdiff = (fborderpx - borderpx) * 2;
		resize(c, c->oldfx, c->oldfy, c->oldfw - borderdiff, c->oldfh - borderdiff, 0);
	} else {
		c->oldfx = c->x;
		c->oldfy = c->y;
		c->oldfw = c->w;
		c->oldfh = c->h;
		XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		c->bw = borderpx;
		configure(c);
	}
	arrange(c->mon);
}

void togglefullscreen(const Arg *arg)
{
	Client *c = selmon->sel;
	if (!c) return;

	if (c->fakefullscreen == 1) { // fake fullscreen --> fullscreen
		c->fakefullscreen = 2;
		setfullscreen(c, 1);
	} else
		setfullscreen(c, !c->isfullscreen);
}

void togglegaps(const Arg *arg)
{
	selmon->pertag->enablegaps[selmon->pertag->curtag] = !selmon->pertag->enablegaps[selmon->pertag->curtag];
	arrange(NULL);
}

void togglescratch(const Arg *arg)
{
	Client *c;
	unsigned int found = 0;

	for (c = selmon->clients; c && !(found = c->tags & scratchtag); c = c->next);
	if (found) {
		unsigned int newtagset = selmon->tagset[selmon->seltags] ^ scratchtag;
		if (newtagset) {
			selmon->tagset[selmon->seltags] = newtagset;
			focus(NULL);
			arrange(selmon);
		}
		if (ISVISIBLE(c)) {
			focus(c);
			restack(selmon);
			XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, WIDTH(c) / 2, HEIGHT(c) / 2);
		}
	} else
		spawn(arg);
}

void togglesticky(const Arg *arg)
{
	if (!selmon->sel) return;
	setsticky(selmon->sel, !selmon->sel->issticky);
	arrange(selmon);
}

void toggletag(const Arg *arg)
{
	unsigned int newtags;

	if (!selmon->sel) return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		selmon->sel->tags = newtags;
		setclienttagprop(selmon->sel);
		focus(NULL);
		arrange(selmon);
	}
	updatecurrentdesktop();
}

void toggleview(const Arg *arg)
{
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);
	int i;

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		if (newtagset == ~0) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			selmon->pertag->curtag = 0;
		}

		/* test if the user did not select the same tag */
		if (!(newtagset & 1 << (selmon->pertag->curtag - 1))) {
			selmon->pertag->prevtag = selmon->pertag->curtag;
			for (i = 0; !(newtagset & 1 << i); i++);
			selmon->pertag->curtag = i + 1;
		}

		/* apply settings for this view */
		selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
		selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
		selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
		selmon->lt[selmon->sellt ^ 1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt ^ 1];

		if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag]) togglebar(NULL);
		focus(NULL);
		arrange(selmon);
	}
	updatecurrentdesktop();
}

void unfocus(Client *c, int setfocus)
{
	if (!c) return;
	grabbuttons(c, 0);
	lastfocused = c;
	if (setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
}

void unmanage(Client *c, int destroyed)
{
	Monitor *m = c->mon;
	XWindowChanges wc;

	detach(c);
	detachstack(c);
	freeicon(c);
	if (!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy); /* avoid race conditions */
		XSetErrorHandler(xerrordummy);
		XSelectInput(dpy, c->win, NoEventMask);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
	if (lastfocused == c) lastfocused = NULL;
	free(c);
	focus(NULL);
	updateclientlist();
	arrange(m);
	if (m == selmon && m->sel) XWarpPointer(dpy, None, m->sel->win, 0, 0, 0, 0, m->sel->w / 2, m->sel->h / 2);
}

void unmapnotify(XEvent *e)
{
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if ((c = wintoclient(ev->window))) {
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, 0);
	}
}

void updatebarpos(Monitor *m)
{
	m->wy = m->my;
	m->wh = m->mh;
	if (m->showbar) {
		m->wh = m->wh - vertpad - bh;
		m->by = m->topbar ? m->wy : m->wy + m->wh + vertpad;
		m->wy = m->topbar ? m->wy + bh + vp : m->wy;
	} else
		m->by = -bh - vp;
}

void updatebars(void)
{
	unsigned int w;
	Monitor *m;
	XSetWindowAttributes wa = { .override_redirect = True,
				    .background_pixmap = ParentRelative,
				    .event_mask = ButtonPressMask | ExposureMask };
	XClassHint ch = { "jdwm", "jdwm" };
	for (m = mons; m; m = m->next) {
		if (m->barwin) continue;
		w = m->ww - 2 * sp;
		if (showsystray && m == systraytomon(m)) w -= getsystraywidth();
		m->barwin = XCreateWindow(dpy, root, m->wx + sp, m->by + vp, w, bh, 0, DefaultDepth(dpy, screen),
					  CopyFromParent, DefaultVisual(dpy, screen),
					  CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
		XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
		if (showsystray && m == systraytomon(m)) XMapRaised(dpy, systray->win);
		XMapRaised(dpy, m->barwin);
		XSetClassHint(dpy, m->barwin, &ch);
	}
}

void updateclientlist(void)
{
	Client *c;
	Monitor *m;

	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
					(unsigned char *)&(c->win), 1);
}

void updatecurrentdesktop(void)
{
	long rawdata[] = { selmon->tagset[selmon->seltags] };
	int i = 0;
	while (*rawdata >> (i + 1)) {
		i++;
	}
	long data[] = { i };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data,
			1);
}

int updategeom(void)
{
	int dirty = 0;

#ifdef XINERAMA
	if (XineramaIsActive(dpy)) {
		int i, j, n, nn;
		Client *c;
		Monitor *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for (n = 0, m = mons; m; m = m->next, n++);
		/* only consider unique geometries as separate screens */
		unique = ecalloc(nn, sizeof(XineramaScreenInfo));
		for (i = 0, j = 0; i < nn; i++)
			if (isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		XFree(info);
		nn = j;

		/* new monitors if nn > n */
		for (i = n; i < nn; i++) {
			for (m = mons; m && m->next; m = m->next);
			if (m)
				m->next = createmon();
			else
				mons = createmon();
		}
		for (i = 0, m = mons; i < nn && m; m = m->next, i++)
			if (i >= n || unique[i].x_org != m->mx || unique[i].y_org != m->my ||
			    unique[i].width != m->mw || unique[i].height != m->mh) {
				dirty = 1;
				m->num = i;
				m->mx = m->wx = unique[i].x_org;
				m->my = m->wy = unique[i].y_org;
				m->mw = m->ww = unique[i].width;
				m->mh = m->wh = unique[i].height;
				updatebarpos(m);
			}
		/* removed monitors if n > nn */
		for (i = nn; i < n; i++) {
			for (m = mons; m && m->next; m = m->next);
			while ((c = m->clients)) {
				dirty = 1;
				m->clients = c->next;
				detachstack(c);
				c->mon = mons;
				attach(c);
				attachstack(c);
			}
			if (m == selmon) selmon = mons;
			cleanupmon(m);
		}
		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		if (!mons) mons = createmon();
		if (mons->mw != sw || mons->mh != sh) {
			dirty = 1;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
			updatebarpos(mons);
		}
	}
	if (dirty) {
		selmon = mons;
		selmon = wintomon(root);
	}
	return dirty;
}

void updateicon(Client *c)
{
	freeicon(c);
	c->icon = geticonprop(c->win, &c->icw, &c->ich);
}

void updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void updatesizehints(Client *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;
	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;
	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;
	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;
	if (size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else
		c->maxa = c->mina = 0.0;
	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
	c->hintsvalid = 1;
}

void updatestatus(void)
{
	if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)) && showstatus) strcpy(stext, "jdwm-" VERSION);
	drawbar(selmon);
	updatesystray();
}

void updatesystray(void)
{
	XSetWindowAttributes wa;
	XWindowChanges wc;
	Client *i;
	Monitor *m = systraytomon(NULL);
	unsigned int x = m->mx + m->mw;
	unsigned int sw = TEXTW(stext) - lrpad + systrayspacing;
	unsigned int w = 1;

	if (!showsystray) return;
	if (systrayonleft) x -= sw + lrpad / 2;
	if (!systray) {
		/* init systray */
		if (!(systray = (Systray *)calloc(1, sizeof(Systray))))
			die("fatal: could not malloc() %u bytes\n", sizeof(Systray));
		systray->win =
			XCreateSimpleWindow(dpy, root, x - sp, m->by + vp, w, bh, 0, 0, scheme[SchemeSel][ColBg].pixel);
		wa.event_mask = ButtonPressMask | ExposureMask;
		wa.override_redirect = True;
		wa.background_pixel = scheme[SchemeSystray][ColBg].pixel;
		XSelectInput(dpy, systray->win, SubstructureNotifyMask);
		XChangeProperty(dpy, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32, PropModeReplace,
				(unsigned char *)&netatom[NetSystemTrayOrientationHorz], 1);
		XChangeWindowAttributes(dpy, systray->win, CWEventMask | CWOverrideRedirect | CWBackPixel, &wa);
		XMapRaised(dpy, systray->win);
		XSetSelectionOwner(dpy, netatom[NetSystemTray], systray->win, CurrentTime);
		if (XGetSelectionOwner(dpy, netatom[NetSystemTray]) == systray->win) {
			sendevent(root, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray],
				  systray->win, 0, 0);
			XSync(dpy, False);
		} else {
			fprintf(stderr, "jdwm: unable to obtain system tray.\n");
			free(systray);
			systray = NULL;
			return;
		}
	}
	for (w = 0, i = systray->icons; i; i = i->next) {
		/* make sure the background color stays the same */
		wa.background_pixel = scheme[SchemeSystray][ColBg].pixel;
		XChangeWindowAttributes(dpy, i->win, CWBackPixel, &wa);
		XMapRaised(dpy, i->win);
		w += systrayspacing;
		i->x = w;
		XMoveResizeWindow(dpy, i->win, i->x, 0, i->w, i->h);
		w += i->w;
		if (i->mon != m) i->mon = m;
	}
	w = w ? w + systrayspacing : 1;
	x -= w;
	XMoveResizeWindow(dpy, systray->win, x - sp, m->by + vp, w, bh);
	wc.x = x - sp;
	wc.y = m->by + vp;
	wc.width = w;
	wc.height = bh;
	wc.stack_mode = Above;
	wc.sibling = m->barwin;
	XConfigureWindow(dpy, systray->win, CWX | CWY | CWWidth | CWHeight | CWSibling | CWStackMode, &wc);
	XMapWindow(dpy, systray->win);
	XMapSubwindows(dpy, systray->win);
	/* redraw background */
	XSetForeground(dpy, drw->gc, scheme[SchemeSystray][ColBg].pixel);
	XFillRectangle(dpy, systray->win, drw->gc, 0, 0, w, bh);
	XSync(dpy, False);
}

void updatesystrayicongeom(Client *i, int w, int h)
{
	if (i) {
		i->h = bh;
		if (w == h)
			i->w = bh;
		else if (h == bh)
			i->w = w;
		else
			i->w = (int)((float)bh * ((float)w / (float)h));
		applysizehints(i, &(i->x), &(i->y), &(i->w), &(i->h), False);
		/* force icons into the systray dimensions if they don't want to */
		if (i->h > bh) {
			if (i->w == i->h)
				i->w = bh;
			else
				i->w = (int)((float)bh * ((float)i->w / (float)i->h));
			i->h = bh;
		}
	}
}

void updatesystrayiconstate(Client *i, XPropertyEvent *ev)
{
	long flags;
	int code = 0;

	if (!showsystray || !i || ev->atom != xatom[XembedInfo] || !(flags = getatomprop(i, xatom[XembedInfo]))) return;

	if (flags & XEMBED_MAPPED && !i->tags) {
		i->tags = 1;
		code = XEMBED_WINDOW_ACTIVATE;
		XMapRaised(dpy, i->win);
		setclientstate(i, NormalState);
	} else if (!(flags & XEMBED_MAPPED) && i->tags) {
		i->tags = 0;
		code = XEMBED_WINDOW_DEACTIVATE;
		XUnmapWindow(dpy, i->win);
		setclientstate(i, WithdrawnState);
	} else
		return;
	sendevent(i->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0, systray->win,
		  XEMBED_EMBEDDED_VERSION);
}

void updatetitle(Client *c)
{
	char oldname[sizeof(c->name)];
	strcpy(oldname, c->name);

	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') { /* hack to mark broken clients */
		strcpy(c->name, broken);
	}
	for (Monitor *m = mons; m; m = m->next) {
		if (m->sel == c && strcmp(oldname, c->name) != 0)
			ipc_focused_title_change_event(m->num, c->win, oldname, c->name);
	}
}

void updatewindowtype(Client *c)
{
	Atom state = getatomprop(c, netatom[NetWMState]);
	Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

	if (state == netatom[NetWMFullscreen]) setfullscreen(c, 1);
	if (state == netatom[NetWMSticky]) {
		setsticky(c, 1);
	}
	if (wtype == netatom[NetWMWindowTypeDialog]) c->isfloating = 1;
}

void updatewmhints(Client *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == selmon->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else
			c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
		if (wmh->flags & InputHint)
			c->neverfocus = !wmh->input;
		else
			c->neverfocus = 0;
		XFree(wmh);
	}
}

void view(const Arg *arg)
{
	int i;
	unsigned int tmptag;
	if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags]) return;
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (arg->ui & TAGMASK) {
		selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
		selmon->pertag->prevtag = selmon->pertag->curtag;

		if (arg->ui == ~0)
			selmon->pertag->curtag = 0;
		else {
			for (i = 0; !(arg->ui & 1 << i); i++);
			selmon->pertag->curtag = i + 1;
		}
	} else {
		tmptag = selmon->pertag->prevtag;
		selmon->pertag->prevtag = selmon->pertag->curtag;
		selmon->pertag->curtag = tmptag;
	}

	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
	selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
	selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
	selmon->lt[selmon->sellt ^ 1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt ^ 1];

	if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag]) togglebar(NULL);
	focus(NULL);
	arrange(selmon);
	updatecurrentdesktop();
}

void viewall(const Arg *arg)
{
	Monitor *m;

	for (m = mons; m; m = m->next) {
		m->tagset[m->seltags] = arg->ui;
		arrange(m);
	}
	focus(NULL);
}

Client *wintoclient(Window w)
{
	Client *c;
	Monitor *m;

	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			if (c->win == w) return c;
	return NULL;
}

Monitor *wintomon(Window w)
{
	int x, y;
	Client *c;
	Monitor *m;

	if (w == root && getrootptr(&x, &y)) return recttomon(x, y, 1, 1);
	for (m = mons; m; m = m->next)
		if (w == m->barwin) return m;
	if ((c = wintoclient(w))) return c->mon;
	return selmon;
}

Client *wintosystrayicon(Window w)
{
	Client *i = NULL;

	if (!showsystray || !w) return i;
	for (i = systray->icons; i && i->win != w; i = i->next);
	return i;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int xerror(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch) ||
	    (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable) ||
	    (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable) ||
	    (ee->request_code == X_PolySegment && ee->error_code == BadDrawable) ||
	    (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch) ||
	    (ee->request_code == X_GrabButton && ee->error_code == BadAccess) ||
	    (ee->request_code == X_GrabKey && ee->error_code == BadAccess) ||
	    (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "jdwm: fatal error: request code=%d, error code=%d\n", ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("jdwm: another window manager is already running");
	return -1;
}

void zoom(const Arg *arg)
{
	Client *c = selmon->sel;

	if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating) return;
	if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next))) return;
	pop(c);
}

// GNU argp parser
const char *argp_program_version = "jdwm " VERSION;
const char *argp_program_bug_address = "https://github.com/JeffofBread/jdwm/issues";
static char doc[] =
	"A custom build of dwm made by JeffofBread. If you wish to know more, check out the github page at https://github.com/JeffofBread/jdwm";
static char args_doc[] = "";
static struct argp_option options[] = {
	{ "first-run", 'F', 0, 0, "Execute dwm along with all programs defined in the array startonce in autostart.h" },
	{ "simple-version", 'v', 0, 0, "Simplified version output" },
	{ 0 }
};
static struct argp argp = { options, parse_opt, args_doc, doc };

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	switch (key) {
	case 'F':
		firstrun = 1;
		break;
	case 'v':
		printf("%s", VERSION);
		exit(EXIT_SUCCESS);
		break;
	case ARGP_KEY_ARG: // Case if too many args
		if (state->arg_num >= 0) argp_usage(state);
		arguments->args[state->arg_num] = arg;
		break;
		//case ARGP_KEY_END: // Case if too few args
		//if (state->arg_num < 2)
		//argp_usage (state);
		//break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	// GNU arg parser
	struct arguments arguments;
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if (firstrun) {
		const char *const *p;
		for (p = autostart; *p; p++) {
			if (fork() == 0) {
				execvp(*p, (char *const *)p);
			}
			while (*++p);
		}
		firstrun = 0;
	}

	// Update wallpaper on reload, meant for theme switching
	if (fork() == 0) {
		execvp(wallpapercmd[0], wallpapercmd);
	}

	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale()) fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL))) die("jdwm: cannot open display");
	checkotherwm();
	setup();
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec", NULL) == -1) die("pledge");
#endif /* __OpenBSD__ */
	scan();
	run();
	char *jdwm_empty_start[] = { "jdwm", NULL };
	if (restart) execvp(argv[0], jdwm_empty_start);
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}

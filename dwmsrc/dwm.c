/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
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
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define INTERSECTC(x,y,w,h,z)   (MAX(0, MIN((x)+(w),(z)->x+(z)->w) - MAX((x),(z)->x)) \
                               * MAX(0, MIN((y)+(h),(z)->y+(z)->h) - MAX((y),(z)->y)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]) || C->issticky)
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TAGSLENGTH              (LENGTH(tags))
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)
#define ColFloat                3

#define SYSTEM_TRAY_REQUEST_DOCK    0
/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY      0
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_FOCUS_IN             4
#define XEMBED_MODALITY_ON         10
#define XEMBED_MAPPED              (1 << 0)
#define XEMBED_WINDOW_ACTIVATE      1
#define XEMBED_WINDOW_DEACTIVATE    2
#define VERSION_MAJOR               0
#define VERSION_MINOR               0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

// Decor hints patch
#define MWM_HINTS_FLAGS_FIELD       0
#define MWM_HINTS_DECORATIONS_FIELD 2
#define MWM_HINTS_DECORATIONS       (1 << 1)
#define MWM_DECOR_ALL               (1 << 0)
#define MWM_DECOR_BORDER            (1 << 1)
#define MWM_DECOR_TITLE             (1 << 3)

/* enums */
enum { CurNormal, CurResize, CurMove, CurSwal, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel, SchemeStatus, SchemeTagsSel, SchemeTagsNorm, SchemeInfoSel, SchemeInfoNorm }; /* color schemes */
enum { NetSupported, NetWMName, NetWMIcon, NetWMState, NetWMCheck,
       NetSystemTray, NetSystemTrayOP, NetSystemTrayOrientation, NetSystemTrayOrientationHorz,
       NetWMFullscreen, NetWMSticky, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetClientList, NetClientInfo, NetDesktopNames, 
       NetDesktopViewport, NetNumberOfDesktops, NetCurrentDesktop, NetLast }; /* EWMH atoms */
enum { Manager, Xembed, XembedInfo, XLast }; /* Xembed atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */
enum { ClientRegular = 1, ClientSwallowee, ClientSwallower }; /* client types */

typedef struct TagState TagState;
struct TagState {
	int selected;
	int occupied;
	int urgent;
};

typedef struct ClientState ClientState;
struct ClientState {
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
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
    float mina, maxa;
    int x, y, w, h;
    int oldx, oldy, oldw, oldh;
    int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
    int bw, oldbw;
    unsigned int tags;
    int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen, ignoretransient, issticky;
    int beingmoved;
    unsigned int icw, ich; Picture icon;
    int fakefullscreen;
    int issteam;
    Client *next;
    Client *snext;
    Client *swer; /* swallower of client, NULL by default */
    Monitor *mon;
    Window win;
    ClientState prevstate;
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
    int by;               /* bar geometry */
    int mx, my, mw, mh;   /* screen size */
    int wx, wy, ww, wh;   /* window area  */
    int gappih;           /* horizontal gap between windows */
    int gappiv;           /* vertical gap between windows */
    int gappoh;           /* horizontal outer gaps */
    int gappov;           /* vertical outer gaps */
    unsigned int seltags;
    unsigned int sellt;
    unsigned int tagset[2];
    TagState tagstate;
    int showbar;
    int topbar;
    Client *clients;
    Client *sel;
    Client *lastsel;
    Client *stack;
    Monitor *next;
    Window barwin;
    const Layout *lt[2];
    const Layout *lastlt;
    int ltcur; /* current layout */
    unsigned int alttag;
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
} Rule;

typedef struct Swallow Swallow;
struct Swallow {
	/* Window class name, instance name (WM_CLASS) and title
	 * (WM_NAME/_NET_WM_NAME, latter preferred if it exists). An empty string
	 * implies a wildcard as per strstr(). */
	char class[256];
	char inst[256];
	char title[256];

	/* Used to delete swallow instance after 'swaldecay' windows were mapped
	 * without the swallow having been consumed. 'decay' keeps track of the
	 * remaining "charges". */
	int decay;

	/* The swallower, i.e. the client which will swallow the next mapped window
	 * whose filters match the above properties. */
	Client *client;

	/* Linked list of registered swallow instances. */
	Swallow *next;
};

typedef struct Systray   Systray;
struct Systray {
	Window win;
	Client *icons;
};

/* function declarations */
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(void);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static void enqueue(Client *c);
static void enqueuestack(Client *c);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static int  fakesignal(void);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static Atom getatomprop(Client *c, Atom prop);
static Picture geticonprop(Window w, unsigned int *icw, unsigned int *ich);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static unsigned int getsystraywidth();
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static int handlexevent(struct epoll_event *ev);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void keyrelease(XEvent *e);
static void killclient(const Arg *arg);
static void losefullscreen(Client *next);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static void moveorplace(const Arg *arg);
static Client *nexttiled(Client *c);
static void placemouse(const Arg *arg);
static void pop(Client *c);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static Client *recttoclient(int x, int y, int w, int h);
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
static void setcurrentdesktop(void);
static void setdesktopnames(void);
static void setclienttagprop(Client *c);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void layoutscroll(const Arg *arg);
static void setsticky(Client *c, int sticky);
static void setlayout(const Arg *arg);
static void setlayoutsafe(const Arg *arg);
static void setmfact(const Arg *arg);
static void setnumdesktops(void);
static void setup(void);
static void setupepoll(void);
static void setviewport(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void sigchld(int unused);
static void sighup(int unused);
static void sigterm(int unused);
static void spawn(const Arg *arg);
static void swal(Client *swer, Client *swee, int manage);
static void swalreg(Client *c, const char* class, const char* inst, const char* title);
static void swaldecayby(int decayby);
static void swalmanage(Swallow *s, Window w, XWindowAttributes *wa);
static Swallow *swalmatch(Window w);
static void swalmouse(const Arg *arg);
static void swalrm(Swallow *s);
static void swalunreg(Client *c);
static void swalstop(Client *c, Client *root);
static void swalstopsel(const Arg *unused);
static Monitor *systraytomon(Monitor *m);
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void togglealttag(const Arg *arg);
static void togglebar(const Arg *arg);
static void togglefakefullscreen(const Arg *arg);
static void togglefloating(const Arg *arg);
static void centerfloating(const Arg *arg);
static void togglescratch(const Arg *arg);
static void togglesticky(const Arg *arg);
static void togglefullscreen(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void freeicon(Client *c);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatecurrentdesktop(void);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updateclientlist(void);
static int updategeom(void);
static void updatemotifhints(Client *c);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updaterules(Client *c);
static void updatesystray(void);
static void updatesystrayicongeom(Client *i, int w, int h);
static void updatesystrayiconstate(Client *i, XPropertyEvent *ev);
static void updatetitle(Client *c);
static void updateicon(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static void viewall(const Arg *arg);
static void warp(const Client *c);
static Client *wintoclient(Window w);
static int wintoclient2(Window w, Client **pc, Client **proot);
static Monitor *wintomon(Window w);
static Client *wintosystrayicon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void zoom(const Arg *arg);
static void autostart_exec(unsigned int firstrun);

/* variables */
static Client *lastfocused = NULL;
static Systray *systray = NULL;
static const char broken[] = "broken";
static char stext[256];
static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh;               /* bar height */
static int lrpad;            /* sum of left and right padding for text */
static int vp;               /* vertical padding for bar */
static int sp;               /* side padding for bar */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
    [ButtonPress] = buttonpress,
    [ClientMessage] = clientmessage,
    [ConfigureRequest] = configurerequest,
    [ConfigureNotify] = configurenotify,
    [DestroyNotify] = destroynotify,
    [EnterNotify] = enternotify,
    [Expose] = expose,
    [FocusIn] = focusin,
    [KeyPress] = keypress,
    [KeyRelease] = keyrelease,
    [MappingNotify] = mappingnotify,
    [MapRequest] = maprequest,
    [MotionNotify] = motionnotify,
    [PropertyNotify] = propertynotify,
    [ResizeRequest] = resizerequest,
    [UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast], xatom[XLast], motifatom;
static int epoll_fd;
static int dpy_fd;
static int restart = 0;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon, *lastselmon;
static Swallow *swallows;
static Window root, wmcheckwin;

#include "ipc.h"

/* configuration, allows nested code to access above variables */
#include <config.h>

#ifdef VERSION
#include "IPCClient.c"
#include "yajl_dumps.c"
#include "ipc.c"
#endif

static unsigned int scratchtag = 1 << LENGTH(tags);

struct Pertag {
    unsigned int curtag, prevtag; /* current and previous tag */
    int nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
    float mfacts[LENGTH(tags) + 1]; /* mfacts per tag */
    unsigned int sellts[LENGTH(tags) + 1]; /* selected layouts */
    const Layout *ltidxs[LENGTH(tags) + 1][2]; /* matrix of tags and layouts indexes  */
    int showbars[LENGTH(tags) + 1]; /* display bar for the current tag */
};

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

/* dwm will keep pid's of processes from autostart array and kill them at quit */
static pid_t *startandrestart_pids;
static size_t startandrestart_len;
static pid_t *startonce_pids;
static size_t startonce_len;

/* execute command from autostart array */
static void
autostart_exec(unsigned int firstrun) {
	const char *const *p;
    const char *const *q;
    struct sigaction sa1;
    struct sigaction sa2;
	size_t i = 0;

	/* count entries */
	for (p = startandrestart; *p; startandrestart_len++, p++)
		while (*++p);
    
    for (q = startonce; *q; startonce_len++, q++)
		while (*++q);

	startandrestart_pids = malloc(startandrestart_len * sizeof(pid_t));
    startonce_pids = malloc(startonce_len * sizeof(pid_t));
	for (p = startandrestart; *p; i++, p++) {
		if ((startandrestart_pids[i] = fork()) == 0) {
			setsid();

			/* Restore SIGCHLD sighandler to default before spawning a program */
			sigemptyset(&sa1.sa_mask);
			sa1.sa_flags = 0;
			sa1.sa_handler = SIG_DFL;
			sigaction(SIGCHLD, &sa1, NULL);

			execvp(*p, (char *const *)p);
			fprintf(stderr, "dwm: execvp %s\n", *p);
			perror(" failed");
			_exit(EXIT_FAILURE);
		}
		/* skip arguments */
		while (*++p);
	}


    if (firstrun == 1){

        i = 0;

        for (q = startonce; *q; i++, q++) {
		    if ((startonce_pids[i] = fork()) == 0) {
			    setsid();

			    /* Restore SIGCHLD sighandler to default before spawning a program */
			    sigemptyset(&sa2.sa_mask);
			    sa2.sa_flags = 0;
			    sa2.sa_handler = SIG_DFL;
			    sigaction(SIGCHLD, &sa2, NULL);

			    execvp(*q, (char *const *)q);
			    fprintf(stderr, "dwm: execvp %s\n", *q);
			    perror(" failed");
			    _exit(EXIT_FAILURE);
		    }
		    /* skip arguments */
		    while (*++q);
	    }
    }
}

/* function implementations */
void
applyrules(Client *c)
{
    const char *class, *instance;
    unsigned int i;
    const Rule *r;
    Monitor *m;
    XClassHint ch = { NULL, NULL };

    /* rule matching */
    c->isfloating = 0;
    c->tags = 0;
    XGetClassHint(dpy, c->win, &ch);
    class    = ch.res_class ? ch.res_class : broken;
    instance = ch.res_name  ? ch.res_name  : broken;

    if (strstr(class, "Steam") || strstr(class, "steam_app_"))
		c->issteam = 1;

    for (i = 0; i < LENGTH(rules); i++) {
        r = &rules[i];
        if ((!r->title || strstr(c->name, r->title))
        && (!r->class || strstr(class, r->class))
        && (!r->instance || strstr(instance, r->instance)))
        {
            c->ignoretransient = r->ignoretransient;
            c->isfloating = r->isfloating;
            c->tags |= r->tags;
            for (m = mons; m && m->num != r->monitor; m = m->next);
            if (m)
                c->mon = m;
        }
    }
    if (ch.res_class)
        XFree(ch.res_class);
    if (ch.res_name)
        XFree(ch.res_name);
    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
    int baseismin;
    Monitor *m = c->mon;

    /* set minimum possible */
    *w = MAX(1, *w);
    *h = MAX(1, *h);
    if (interact) {
        if (*x > sw)
            *x = sw - WIDTH(c);
        if (*y > sh)
            *y = sh - HEIGHT(c);
        if (*x + *w + 2 * c->bw < 0)
            *x = 0;
        if (*y + *h + 2 * c->bw < 0)
            *y = 0;
    } else {
        if (*x >= m->wx + m->ww)
            *x = m->wx + m->ww - WIDTH(c);
        if (*y >= m->wy + m->wh)
            *y = m->wy + m->wh - HEIGHT(c);
        if (*x + *w + 2 * c->bw <= m->wx)
            *x = m->wx;
        if (*y + *h + 2 * c->bw <= m->wy)
            *y = m->wy;
    }
    if (*h < bh)
        *h = bh;
    if (*w < bh)
        *w = bh;
    if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
        if (!c->hintsvalid)
            updatesizehints(c);
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
        if (c->incw)
            *w -= *w % c->incw;
        if (c->inch)
            *h -= *h % c->inch;
        /* restore base dimensions */
        *w = MAX(*w + c->basew, c->minw);
        *h = MAX(*h + c->baseh, c->minh);
        if (c->maxw)
            *w = MIN(*w, c->maxw);
        if (c->maxh)
            *h = MIN(*h, c->maxh);
    }
    return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void
arrange(Monitor *m)
{
    if (m)
        showhide(m->stack);
    else for (m = mons; m; m = m->next)
        showhide(m->stack);
    if (m) {
        arrangemon(m);
        restack(m);
    } else for (m = mons; m; m = m->next)
        arrangemon(m);
}

void
arrangemon(Monitor *m)
{
    strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
    if (m->lt[m->sellt]->arrange)
        m->lt[m->sellt]->arrange(m);
}

void
attach(Client *c)
{
    c->next = c->mon->clients;
    c->mon->clients = c;
}

void
attachstack(Client *c)
{
    c->snext = c->mon->stack;
    c->mon->stack = c;
}

void
buttonpress(XEvent *e)
{
    unsigned int i, x, click;
    Arg arg = {0};
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
        for(c = m->clients; c; c=c->next)
            occ |= c->tags;
        do {
            /* Do not reserve space for vacant tags */
            if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
                continue;
		    if (showtags)
				x += TEXTW(tags[i]);
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
        if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
        && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
            buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void
checkotherwm(void)
{
    xerrorxlib = XSetErrorHandler(xerrorstart);
    /* this causes an error if some other window manager is running */
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XSync(dpy, False);
}

void
cleanup(void)
{
    Arg a = {.ui = ~0};
    Layout foo = { "", NULL };
    Monitor *m;
    size_t i;

    view(&a);
    selmon->lt[selmon->sellt] = &foo;
    for (m = mons; m; m = m->next)
        while (m->stack)
            unmanage(m->stack, 0);
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    while (mons) {
        cleanupmon(mons);
    }
	if (showsystray) {
		XUnmapWindow(dpy, systray->win);
		XDestroyWindow(dpy, systray->win);
		free(systray);
	}
    for (i = 0; i < CurLast; i++)
        drw_cur_free(drw, cursor[i]);
    for (i = 0; i < LENGTH(colors); i++)
        free(scheme[i]);
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

void
cleanupmon(Monitor *mon)
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

void
clientmessage(XEvent *e)
{
	XWindowAttributes wa;
	XSetWindowAttributes swa;
    XClientMessageEvent *cme = &e->xclient;
    Client *c = wintoclient(cme->window);
    unsigned int i;

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
            XClassHint ch = {"dwmsystray", "dwmsystray"};
			XSetClassHint(dpy, c->win, &ch);

			XReparentWindow(dpy, c->win, systray->win, 0, 0);
			/* use parents background color */
			swa.background_pixel  = scheme[SchemeNorm][ColBg].pixel;
			XChangeWindowAttributes(dpy, c->win, CWBackPixel, &swa);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			/* FIXME not sure if I have to send these events, too */
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_FOCUS_IN, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_MODALITY_ON, 0 , systray->win, XEMBED_EMBEDDED_VERSION);
			XSync(dpy, False);
			resizebarwin(selmon);
			updatesystray();
			setclientstate(c, NormalState);
		}
		return;
	}

    if (!c)
        return;
    if (cme->message_type == netatom[NetWMState]) {
        if (cme->data.l[1] == netatom[NetWMFullscreen]
        || cme->data.l[2] == netatom[NetWMFullscreen]) {
            if (c->fakefullscreen == 2 && c->isfullscreen)
                c->fakefullscreen = 3;
            setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
                || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));

        if (cme->data.l[1] == netatom[NetWMSticky]
                || cme->data.l[2] == netatom[NetWMSticky])
            setsticky(c, (cme->data.l[0] == 1 || (cme->data.l[0] == 2 && !c->issticky)));
        }
    } else if (cme->message_type == netatom[NetActiveWindow]) {
		for (i = 0; i < LENGTH(tags) && !((1 << i) & c->tags); i++);
		if (i < LENGTH(tags)) {
			const Arg a = {.ui = 1 << i};
			selmon = c->mon;
			view(&a);
			focus(c);
			restack(selmon);
            XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);
		}
    }
}

void
configure(Client *c)
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

void
configurenotify(XEvent *e)
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

void
configurerequest(XEvent *e)
{
    Client *c;
    Monitor *m;
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;

	switch (wintoclient2(ev->window, &c, NULL)) {
	case ClientRegular: /* fallthrough */
	case ClientSwallowee:
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
            if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
                configure(c);
            if (ISVISIBLE(c))
                XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
        } else
            configure(c);
		break;
	    case ClientSwallower:
		    /* Reject any move/resize requests for swallowers and communicate
		     * refusal to client via a synthetic ConfigureNotify (ICCCM 4.1.5). */
		    configure(c);
		    break;
	    default:
            wc.x = ev->x;
            wc.y = ev->y;
            wc.width = ev->width;
            wc.height = ev->height;
            wc.border_width = ev->border_width;
            wc.sibling = ev->above;
            wc.stack_mode = ev->detail;
            XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
            break;
        }
    XSync(dpy, False);
}

Monitor *
createmon(void)
{
    Monitor *m;
    unsigned int i;

    m = ecalloc(1, sizeof(Monitor));
    m->tagset[0] = m->tagset[1] = 1;
    m->mfact = mfact;
    m->nmaster = nmaster;
    m->showbar = showbar;
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
    }
    return m;
}

void
destroynotify(XEvent *e)
{
    Client *c, *swee, *root;
    XDestroyWindowEvent *ev = &e->xdestroywindow;
    if ((c = wintosystrayicon(ev->window))) {
		removesystrayicon(c);
		resizebarwin(selmon);
		updatesystray();
	}
	switch (wintoclient2(ev->window, &c, &root)) {
	case ClientRegular:
		unmanage(c, 1);
		break;
	case ClientSwallowee:
		swalstop(c, NULL);
		unmanage(c, 1);
		break;
	case ClientSwallower:
		/* If the swallower is swallowed by another client, terminate the
		 * swallow. This cuts off the swallow chain after the client. */
		swalstop(c, root);

		/* Cut off the swallow chain before the client. */
		for (swee = root; swee->swer != c; swee = swee->swer);
		swee->swer = NULL;

		free(c);
		updateclientlist();
		break;
	}
}

void
detach(Client *c)
{
    Client **tc;

    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;
}

void
detachstack(Client *c)
{
    Client **tc, *t;

    for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
    *tc = c->snext;

    if (c == c->mon->sel) {
        for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
        c->mon->sel = t;
    }
}

Monitor *
dirtomon(int dir)
{
    Monitor *m = NULL;

    if (dir > 0) {
        if (!(m = selmon->next))
            m = mons;
    } else if (selmon == mons)
        for (m = mons; m->next; m = m->next);
    else
        for (m = mons; m->next != selmon; m = m->next);
    return m;
}

void
drawbar(Monitor *m)
{
    int x, w, wdelta, tw = 0, stw = 0;
    int boxs = drw->fonts->h / 9;
    int boxw = drw->fonts->h / 6 + 2;
    unsigned int i, occ = 0, urg = 0;
    Client *c;

    if (!m->showbar) {
        return;
    }

	if(showsystray && m == systraytomon(m) && !systrayonleft)
		stw = getsystraywidth();

    /* draw status first so it can be overdrawn by tags later */
    if (m == selmon && showstatus) { /* status is only drawn on selected monitor */
        drw_setscheme(drw, scheme[SchemeStatus]);
		tw = TEXTW(stext) - lrpad / 2;
        drw_text(drw, m->ww - tw - stw - 2 * sp, 0, tw, bh, lrpad / 2, stext, 0);
    }

    resizebarwin(m);
    for (c = m->clients; c; c = c->next) {
        occ |= c->tags;
        if (c->isurgent && showtags)
            urg |= c->tags;
    }
    x = 0;
    for (i = 0; i < LENGTH(tags); i++) {
        /* Do not draw vacant tags */
        if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
            continue;
		if (showtags) {
				w = TEXTW(tags[i]);
                wdelta = selmon->alttag ? abs(TEXTW(tags[i]) - TEXTW(tagsalt[i])) / 2 : 0;
				drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeTagsSel : SchemeTagsNorm]);
                drw_text(drw, x, 0, w, bh, wdelta + lrpad / 2, (selmon->alttag ? tagsalt[i] : tags[i]), urg & 1 << i);
		        if (ulineall || m->tagset[m->seltags] & 1 << i) /* if there are conflicts, just move these lines directly underneath both 'drw_setscheme' and 'drw_text' :) */
			        drw_rect(drw, x + ulinepad, bh - ulinestroke - ulinevoffset, w - (ulinepad * 2), ulinestroke, 1, 0);
                if (occ & 1 << i && showfloating){
                    // idk how this fixes vacant tabs + bartoggle, but it does - JeffofBread
				    drw_rect(0, 0, 0, 0, 0, m == selmon && selmon->sel && selmon->sel->tags & 1 << i, urg & 1 << i);
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

	/* Draw swalsymbol next to ltsymbol. */
	if (m->sel && m->sel->swer) {
		w = TEXTW(swalsymbol);
		x = drw_text(drw, x, 0, w, bh, lrpad / 2, swalsymbol, 0);
	}

    if ((w = m->ww - tw - stw - x) > bh) {
        if (m->sel && showtitle) {
            /* fix overflow when window name is bigger than window width */
			int mid = (m->ww - (int)TEXTW(m->sel->name)) / 2 - x;
			/* make sure name will not overlap on tags even when it is very long */
			mid = mid >= lrpad / 2 ? mid : lrpad / 2;
            drw_setscheme(drw, scheme[m == selmon ? SchemeInfoSel : SchemeInfoNorm]);
            if (centeredwindowname == 0) {
                drw_text(drw, x, 0, w - 2 * sp, bh, lrpad / 2 + (m->sel->icon ? m->sel->icw + ICONSPACING : 0), m->sel->name, 0);
                if (m->sel->icon) drw_pic(drw, x + lrpad / 2, (bh - m->sel->ich) / 2, m->sel->icw, m->sel->ich, m->sel->icon);
            } 
            else if (centeredwindowname == 1) {
                drw_text(drw, x, 0, w - 2 * sp, bh, mid + (m->sel->icon ? m->sel->icw + ICONSPACING : 0), m->sel->name, 0);
                if (m->sel->icon) drw_pic(drw, x + lrpad / 2, (bh - m->sel->ich) / 2, m->sel->icw, m->sel->ich, m->sel->icon);
            }
            if (m->sel->isfloating && showfloating)
                drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
            if (m->sel->issticky)
				drw_polygon(drw, x + boxs, m->sel->isfloating ? boxs * 2 + boxw : boxs, stickyiconbb.x, stickyiconbb.y, boxw, boxw * stickyiconbb.y / stickyiconbb.x, stickyicon, LENGTH(stickyicon), Nonconvex, m->sel->tags & m->tagset[m->seltags]);
        } else {
            drw_setscheme(drw, scheme[SchemeInfoNorm]);
            drw_rect(drw, x, 0, w - 2 * sp, bh, 1, 1);
        }
    }
    drw_map(drw, m->barwin, 0, 0, m->ww - stw, bh);
}

void
drawbars(void)
{
    Monitor *m;

    for (m = mons; m; m = m->next)
        drawbar(m);
}

void
enqueue(Client *c)
{
	Client *l;
	for (l = c->mon->clients; l && l->next; l = l->next);
	if (l) {
		l->next = c;
		c->next = NULL;
	}
}

void
enqueuestack(Client *c)
{
	Client *l;
	for (l = c->mon->stack; l && l->snext; l = l->snext);
	if (l) {
		l->snext = c;
		c->snext = NULL;
	}
}

void
enternotify(XEvent *e)
{
    Client *c;
    Monitor *m;
    XCrossingEvent *ev = &e->xcrossing;

    if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
        return;
    c = wintoclient(ev->window);
    m = c ? c->mon : wintomon(ev->window);
    if (m != selmon) {
        unfocus(selmon->sel, 1);
        selmon = m;
    } else if (!c || c == selmon->sel)
        return;
    focus(c);
}

void
expose(XEvent *e)
{
    Monitor *m;
    XExposeEvent *ev = &e->xexpose;

    if (ev->count == 0 && (m = wintomon(ev->window))) {
        drawbar(m);
		if (m == selmon)
			updatesystray();
	}
}

int
fakesignal(void)
{
	/* Command syntax: <PREFIX><COMMAND>[<SEP><ARG>]... */
	static const char sep[] = "###";
	static const char prefix[] = "#!";

	size_t numsegments, numargs;
	char rootname[256];
	char *segments[16] = {0};

	/* Get root name, split by separator and find the prefix */
	if (!gettextprop(root, XA_WM_NAME, rootname, sizeof(rootname))
		|| strncmp(rootname, prefix, sizeof(prefix) - 1)) {
		return 0;
	}
	numsegments = split(rootname + sizeof(prefix) - 1, sep, segments, sizeof(segments));
	numargs = numsegments - 1; /* number of arguments to COMMAND */

	if (!strcmp(segments[0], "swalreg")) {
		/* Params: windowid, [class], [instance], [title] */
		Window w;
		Client *c;

		if (numargs >= 1) {
			w = strtoul(segments[1], NULL, 0);
			switch (wintoclient2(w, &c, NULL)) {
			case ClientRegular: /* fallthrough */
			case ClientSwallowee:
				swalreg(c, segments[2], segments[3], segments[4]);
				break;
			}
		}
	}
	else if (!strcmp(segments[0], "swal")) {
		/* Params: swallower's windowid, swallowee's window-id */
		Client *swer, *swee;
		Window winswer, winswee;
		int typeswer, typeswee;

		if (numargs >= 2) {
			winswer = strtoul(segments[1], NULL, 0);
			typeswer = wintoclient2(winswer, &swer, NULL);
			winswee = strtoul(segments[2], NULL, 0);
			typeswee = wintoclient2(winswee, &swee, NULL);
			if ((typeswer == ClientRegular || typeswer == ClientSwallowee)
				&& (typeswee == ClientRegular || typeswee == ClientSwallowee))
				swal(swer, swee, 0);
		}
	}
	else if (!strcmp(segments[0], "swalunreg")) {
		/* Params: swallower's windowid */
		Client *swer;
		Window winswer;

		if (numargs == 1) {
			winswer = strtoul(segments[1], NULL, 0);
			if ((swer = wintoclient(winswer)))
				swalunreg(swer);
		}
	}
	else if (!strcmp(segments[0], "swalstop")) {
		/* Params: swallowee's windowid */
		Client *swee;
		Window winswee;

		if (numargs == 1) {
			winswee = strtoul(segments[1], NULL, 0);
			if ((swee = wintoclient(winswee)))
				swalstop(swee, NULL);
		}
	}
	return 1;
}

void
focus(Client *c)
{
    if (!c || !ISVISIBLE(c))
        for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
    if (selmon->sel && selmon->sel != c) {
        losefullscreen(c);
        unfocus(selmon->sel, 0);
    }
    if (c) {
        if (c->mon != selmon)
            selmon = c->mon;
        if (c->isurgent)
            seturgent(c, 0);
        detachstack(c);
        attachstack(c);
        grabbuttons(c, 1);
        /* set new focused border first to avoid flickering */
        if(c->isfloating){
			XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColFloat].pixel);
            if(raisefloatwinfoc)
                XRaiseWindow(dpy, c->win);
        }
		else
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
void
focusin(XEvent *e)
{
    XFocusChangeEvent *ev = &e->xfocus;

    if (selmon->sel && ev->window != selmon->sel->win)
        setfocus(selmon->sel);
}

void
focusmon(const Arg *arg)
{
    Monitor *m;

    if (!mons->next)
        return;
    if ((m = dirtomon(arg->i)) == selmon)
        return;
    unfocus(selmon->sel, 0);
    selmon = m;
    focus(NULL);
    
    // Thank you to u/bakkeby on r/dwm for the solution, which was derived from the cursor warp patch
    if (selmon->sel)
        XWarpPointer(dpy, None, selmon->sel->win, 0, 0, 0, 0, selmon->sel->w/2, selmon->sel->h/2);
    else 
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->wx + selmon->ww/2, selmon->wy + selmon->wh/2);
}

void
focusstack(const Arg *arg)
{
    Client *c = NULL, *i;

    /* Note that this patch is made for dwm 6.2. If applying this on top of the latest master
     * then the https://dwm.suckless.org/patches/alwaysfullscreen/ patch has been merged
     * upstream, which prevents focus to drift from windows that are in fake fullscreen.
     * To address this issue use this if statement instead:
     *   if (!selmon->sel || (selmon->sel->isfullscreen && selmon->sel->fakefullscreen != 1))
     */

    if (!selmon->sel || (selmon->sel->isfullscreen && selmon->sel->fakefullscreen != 1))
        return;
    if (arg->i > 0) {
        for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
        if (!c)
            for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
    } else {
        for (i = selmon->clients; i != selmon->sel; i = i->next)
            if (ISVISIBLE(i))
                c = i;
        if (!c)
            for (; i; i = i->next)
                if (ISVISIBLE(i))
                    c = i;
    }
    if (c) {
        focus(c);
        restack(selmon);
        XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);
    }
}

Atom
getatomprop(Client *c, Atom prop)
{
    int di;
    unsigned long dl;
    unsigned char *p = NULL;
    Atom da, atom = None;

    /* FIXME getatomprop should return the number of items and a pointer to
	 * the stored data instead of this workaround */
	Atom req = XA_ATOM;
	if (prop == xatom[XembedInfo])
		req = xatom[XembedInfo];

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, req,
        &da, &di, &dl, &dl, &p) == Success && p) {
        atom = *(Atom *)p;
		if (da == xatom[XembedInfo] && dl == 2)
			atom = ((Atom *)p)[1];
        XFree(p);
    }
    return atom;
}

static uint32_t prealpha(uint32_t p) {
	uint8_t a = p >> 24u;
	uint32_t rb = (a * (p & 0xFF00FFu)) >> 8u;
	uint32_t g = (a * (p & 0x00FF00u)) >> 8u;
	return (rb & 0xFF00FFu) | (g & 0x00FF00u) | (a << 24u);
}

Picture
geticonprop(Window win, unsigned int *picw, unsigned int *pich)
{
	int format;
	unsigned long n, extra, *p = NULL;
	Atom real;

	if (XGetWindowProperty(dpy, win, netatom[NetWMIcon], 0L, LONG_MAX, False, AnyPropertyType, 
						   &real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return None; 
	if (n == 0 || format != 32) { XFree(p); return None; }

	unsigned long *bstp = NULL;
	uint32_t w, h, sz;
	{
		unsigned long *i; const unsigned long *end = p + n;
		uint32_t bstd = UINT32_MAX, d, m;
		for (i = p; i < end - 1; i += sz) {
			if ((w = *i++) >= 16384 || (h = *i++) >= 16384) { XFree(p); return None; }
			if ((sz = w * h) > end - i) break;
			if ((m = w > h ? w : h) >= ICONSIZE && (d = m - ICONSIZE) < bstd) { bstd = d; bstp = i; }
		}
		if (!bstp) {
			for (i = p; i < end - 1; i += sz) {
				if ((w = *i++) >= 16384 || (h = *i++) >= 16384) { XFree(p); return None; }
				if ((sz = w * h) > end - i) break;
				if ((d = ICONSIZE - (w > h ? w : h)) < bstd) { bstd = d; bstp = i; }
			}
		}
		if (!bstp) { XFree(p); return None; }
	}

	if ((w = *(bstp - 2)) == 0 || (h = *(bstp - 1)) == 0) { XFree(p); return None; }

	uint32_t icw, ich;
	if (w <= h) {
		ich = ICONSIZE; icw = w * ICONSIZE / h;
		if (icw == 0) icw = 1;
	}
	else {
		icw = ICONSIZE; ich = h * ICONSIZE / w;
		if (ich == 0) ich = 1;
	}
	*picw = icw; *pich = ich;

	uint32_t i, *bstp32 = (uint32_t *)bstp;
	for (sz = w * h, i = 0; i < sz; ++i) bstp32[i] = prealpha(bstp[i]);

	Picture ret = drw_picture_create_resized(drw, (char *)bstp, w, h, icw, ich);
	XFree(p);

	return ret;
}

unsigned int
getsystraywidth()
{
	unsigned int w = 0;
	Client *i;
	if(showsystray)
		for(i = systray->icons; i; w += i->w + systrayspacing, i = i->next) ;
	return w ? w + systrayspacing : 1;
}

int
getrootptr(int *x, int *y)
{
    int di;
    unsigned int dui;
    Window dummy;

    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long
getstate(Window w)
{
    int format;
    long result = -1;
    unsigned char *p = NULL;
    unsigned long n, extra;
    Atom real;

    if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
        &real, &format, &n, &extra, (unsigned char **)&p) != Success)
        return -1;
    if (n != 0)
        result = *p;
    XFree(p);
    return result;
}

int
gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
    char **list = NULL;
    int n;
    XTextProperty name;

    if (!text || size == 0)
        return 0;
    text[0] = '\0';
    if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
        return 0;
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

void
grabbuttons(Client *c, int focused)
{
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        if (!focused)
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
                BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
        for (i = 0; i < LENGTH(buttons); i++)
            if (buttons[i].click == ClkClientWin)
                for (j = 0; j < LENGTH(modifiers); j++)
                    XGrabButton(dpy, buttons[i].button,
                        buttons[i].mask | modifiers[j],
                        c->win, False, BUTTONMASK,
                        GrabModeAsync, GrabModeSync, None, None);
    }
}

void
grabkeys(void)
{
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
        KeyCode code;

        XUngrabKey(dpy, AnyKey, AnyModifier, root);
        for (i = 0; i < LENGTH(keys); i++)
            if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
                for (j = 0; j < LENGTH(modifiers); j++)
                    XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
                        True, GrabModeAsync, GrabModeAsync);
    }
}

int
handlexevent(struct epoll_event *ev)
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
	} else if (ev-> events & EPOLLHUP) {
		return -1;
	}

	return 0;
}

void
incnmaster(const Arg *arg)
{
    selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = MAX(selmon->nmaster + arg->i, 0);
    arrange(selmon);
}

#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
    while (n--)
        if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
        && unique[n].width == info->width && unique[n].height == info->height)
            return 0;
    return 1;
}
#endif /* XINERAMA */

void
keypress(XEvent *e)
{
    unsigned int i;
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
    keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
    for (i = 0; i < LENGTH(keys); i++)
        if (keysym == keys[i].keysym
        && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
        && keys[i].func)
            keys[i].func(&(keys[i].arg));
}

void
keyrelease(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);

    for (i = 0; i < LENGTH(keys); i++)
        if (momentaryalttags
        && keys[i].func && keys[i].func == togglealttag
        && selmon->alttag
        && (keysym == keys[i].keysym
        || CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)))
            keys[i].func(&(keys[i].arg));
}

void
killclient(const Arg *arg)
{
    if (!selmon->sel)
        return;
    if (!sendevent(selmon->sel->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0 , 0, 0)) {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, selmon->sel->win);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
}

void
losefullscreen(Client *next)
{
    Client *sel = selmon->sel;
    if (!sel || !next)
        return;
    if (sel->isfullscreen && sel->fakefullscreen != 1 && ISVISIBLE(sel) && sel->mon == next->mon && !next->isfloating)
        setfullscreen(sel, 0);
}

void
manage(Window w, XWindowAttributes *wa)
{
    Client *c, *t = NULL;
    Window trans = None;
    XWindowChanges wc;

    c = ecalloc(1, sizeof(Client));
    c->win = w;
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
        updatewindowtype(c);
    } else {
        c->mon = selmon;
        applyrules(c);
        updatewindowtype(c);
        {
            int format;
            unsigned long *data, n, extra;
            Monitor *m;
            Atom atom;
            if (XGetWindowProperty(dpy, c->win, netatom[NetClientInfo], 0L, 2L, False, XA_CARDINAL,
                    &atom, &format, &n, &extra, (unsigned char **)&data) == Success && n == 2) {
                c->tags = *data;
                for (m = mons; m; m = m->next) {
                    if (m->num == *(data+1)) {
                        c->mon = m;
                        break;
                    }
                }
            }
            if (n > 0)
                XFree(data);
        }
        setclienttagprop(c);
    }

    if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww){ c->x = c->mon->wx + c->mon->ww - WIDTH(c); }
    if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh){ c->y = c->mon->wy + c->mon->wh - HEIGHT(c); }

	selmon->tagset[selmon->seltags] &= ~scratchtag;
	if (!strcmp(c->name, scratchpadname)) {
		c->mon->tagset[c->mon->seltags] |= c->tags = scratchtag;
		c->isfloating = True;
	}


    if (!c->isfloating) 
        c->isfloating = c->oldstate = trans != None || c->isfixed;

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
    c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;/*  */
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);
    configure(c); /* propagates border_width, if size doesn't change */
    updatemotifhints(c);
    XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    grabbuttons(c, 0);
    attach(c);
    attachstack(c);
    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
        (unsigned char *) &(c->win), 1);
    XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
    setclientstate(c, NormalState);
    if (c->mon == selmon) {
        losefullscreen(c);
        unfocus(selmon->sel, 0);
    }
    c->mon->sel = c;
    arrange(c->mon);
    XMapWindow(dpy, c->win);
    if (c && c->mon == selmon)
        XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);
    focus(NULL);
}

void
mappingnotify(XEvent *e)
{
    XMappingEvent *ev = &e->xmapping;

    XRefreshKeyboardMapping(ev);
    if (ev->request == MappingKeyboard)
        grabkeys();
}

void
maprequest(XEvent *e)
{
    Client *c, *swee, *root;
    static XWindowAttributes wa;
    XMapRequestEvent *ev = &e->xmaprequest;
    Swallow *s;

	Client *i;
	if ((i = wintosystrayicon(ev->window))) {
		sendevent(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
		resizebarwin(selmon);
		updatesystray();
	}

    if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect) {
        return;
    }
	switch (wintoclient2(ev->window, &c, &root)) {
	case ClientRegular: /* fallthrough */
	case ClientSwallowee:
		/* Regulars and swallowees are always mapped. Nothing to do. */
		break;
	case ClientSwallower:
		/* Remapping a swallower will simply stop the swallow. */
		for (swee = root; swee->swer != c; swee = swee->swer);
		swalstop(swee, root);
		break;
	default:
		/* No client is managing the window. See if any swallows match. */
		if ((s = swalmatch(ev->window)))
			swalmanage(s, ev->window, &wa);
		else
			manage(ev->window, &wa);
		break;
	}

	/* Reduce decay counter of all swallow instances. */
	if (swaldecay)
		swaldecayby(1);
}

void
motionnotify(XEvent *e)
{
    static Monitor *mon = NULL;
    Monitor *m;
    XMotionEvent *ev = &e->xmotion;
    if (ev->window != root)
        return;
    if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    mon = m;
}

void
moveorplace(const Arg *arg) {
	if ((!selmon->lt[selmon->sellt]->arrange || (selmon->sel && selmon->sel->isfloating)))
		movemouse(arg);
	else
		placemouse(arg);
}

void
movemouse(const Arg *arg)
{
    int x, y, ocx, ocy, nx, ny;
    Client *c;
    Monitor *m;
    XEvent ev;
    Time lasttime = 0;

    if (!(c = selmon->sel))
        return;
    if (c->isfullscreen && c->fakefullscreen != 1) /* no support moving fullscreen windows by mouse */
        return;
    restack(selmon);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
        None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
        return;
    if (!getrootptr(&x, &y))
        return;
    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 60))
                continue;
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
            if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
            && (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
                togglefloating(NULL);
            if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                resize(c, nx, ny, c->w, c->h, 1);
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

Client *
nexttiled(Client *c)
{
    for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
    return c;
}

void
placemouse(const Arg *arg)
{
	int x, y, px, py, ocx, ocy, nx = -9999, ny = -9999, freemove = 0;
	Client *c, *r = NULL, *at, *prevr;
	Monitor *m;
	XEvent ev;
	XWindowAttributes wa;
	Time lasttime = 0;
	int attachmode, prevattachmode;
	attachmode = prevattachmode = -1;

	if (!(c = selmon->sel) || !c->mon->lt[c->mon->sellt]->arrange) /* no support for placemouse when floating layout is used */
		return;
	if (c->isfullscreen) /* no support placing fullscreen windows by mouse */
		return;
	restack(selmon);
	prevr = c;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

	c->isfloating = 0;
	c->beingmoved = 1;

	XGetWindowAttributes(dpy, c->win, &wa);
	ocx = wa.x;
	ocy = wa.y;

	if (arg->i == 2) // warp cursor to client center
		XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, WIDTH(c) / 2, HEIGHT(c) / 2);

	if (!getrootptr(&x, &y))
		return;

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);

			if (!freemove && (abs(nx - ocx) > snap || abs(ny - ocy) > snap))
				freemove = 1;

			if (freemove)
				XMoveWindow(dpy, c->win, nx, ny);

			if ((m = recttomon(ev.xmotion.x, ev.xmotion.y, 1, 1)) && m != selmon)
				selmon = m;

			if (arg->i == 1) { // tiled position is relative to the client window center point
				px = nx + wa.width / 2;
				py = ny + wa.height / 2;
			} else { // tiled position is relative to the mouse cursor
				px = ev.xmotion.x;
				py = ev.xmotion.y;
			}

			r = recttoclient(px, py, 1, 1);

			if (!r || r == c)
				break;

			attachmode = 0; // below
			if (((float)(r->y + r->h - py) / r->h) > ((float)(r->x + r->w - px) / r->w)) {
				if (abs(r->y - py) < r->h / 2)
					attachmode = 1; // above
			} else if (abs(r->x - px) < r->w / 2)
				attachmode = 1; // above

			if ((r && r != prevr) || (attachmode != prevattachmode)) {
				detachstack(c);
				detach(c);
				if (c->mon != r->mon) {
					arrangemon(c->mon);
					c->tags = r->mon->tagset[r->mon->seltags];
				}

				c->mon = r->mon;
				r->mon->sel = r;

				if (attachmode) {
					if (r == r->mon->clients)
						attach(c);
					else {
						for (at = r->mon->clients; at->next != r; at = at->next);
						c->next = at->next;
						at->next = c;
					}
				} else {
					c->next = r->next;
					r->next = c;
				}

				attachstack(c);
				arrangemon(r->mon);
				prevr = r;
				prevattachmode = attachmode;
			}
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);

	if ((m = recttomon(ev.xmotion.x, ev.xmotion.y, 1, 1)) && m != c->mon) {
		detach(c);
		detachstack(c);
		arrangemon(c->mon);
		c->mon = m;
		c->tags = m->tagset[m->seltags];
		attach(c);
		attachstack(c);
		selmon = m;
	}

	focus(c);
	c->beingmoved = 0;

	if (nx != -9999)
		resize(c, nx, ny, c->w, c->h, 0);
	arrangemon(c->mon);
    arrange(c->mon);
}

void
pop(Client *c)
{
    detach(c);
    attach(c);
    focus(c);
    arrange(c->mon);
}

void
propertynotify(XEvent *e)
{
    Client *c;
    Window trans;
    Swallow *s;
    XPropertyEvent *ev = &e->xproperty;

	if ((c = wintosystrayicon(ev->window))) {
    	if (ev->atom == XA_WM_NORMAL_HINTS) {
			updatesizehints(c);
			updatesystrayicongeom(c, c->w, c->h);
		}
		else
			updatesystrayiconstate(c, ev);
		resizebarwin(selmon);
		updatesystray();
	}

	if ((ev->window == root) && (ev->atom == XA_WM_NAME)) {
		if (!fakesignal())
			updatestatus();
	} else if (ev->state == PropertyDelete)
        return; /* ignore */
    else if ((c = wintoclient(ev->window))) {
        switch(ev->atom) {
        default: break;
        case XA_WM_TRANSIENT_FOR:
			if (!c->ignoretransient && !c->isfloating && 
			(XGetTransientForHint(dpy, c->win, &trans)) && 
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
            updaterules(c);
            if (c == c->mon->sel && showtitle)
                drawbar(c->mon);
        }
        else if (ev->atom == netatom[NetWMIcon]) {
			updateicon(c);
			if (c == c->mon->sel)
				drawbar(c->mon);
            if (swalretroactive && (s = swalmatch(c->win)))
				swal(s->client, c, 0);
		}
        if (ev->atom == netatom[NetWMWindowType])
            updatewindowtype(c);
        if (ev->atom == motifatom)
            updatemotifhints(c);
    }
}

void
quit(const Arg *arg)
{
	size_t i;

	/* kill child processes */
	for (i = 0; i < startandrestart_len; i++) {
		if (0 < startandrestart_pids[i]) {
			kill(startandrestart_pids[i], SIGTERM);
			waitpid(startandrestart_pids[i], NULL, 0);
		}
	}
    if(arg->i) restart = 1;
    running = 0;
}

Client *
recttoclient(int x, int y, int w, int h)
{
	Client *c, *r = NULL;
	int a, area = 0;

	for (c = nexttiled(selmon->clients); c; c = nexttiled(c->next)) {
		if ((a = INTERSECTC(x, y, w, h, c)) > area) {
			area = a;
			r = c;
		}
	}
	return r;
}

Monitor *
recttomon(int x, int y, int w, int h)
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

void
removesystrayicon(Client *i)
{
	Client **ii;

    XReparentWindow(dpy, i->win, root, 0, 0);  // https://www.reddit.com/r/suckless/comments/vdpq14/comment/icpsnhe/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
	if (!showsystray || !i)
		return;
	for (ii = &systray->icons; *ii && *ii != i; ii = &(*ii)->next);
	if (ii)
		*ii = i->next;
	free(i);
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
    if (applysizehints(c, &x, &y, &w, &h, interact))
        resizeclient(c, x, y, w, h);
}

void
resizebarwin(Monitor *m) {
	unsigned int w = m->ww -  2 * sp;
	if (showsystray && m == systraytomon(m) && !systrayonleft)
		w -= getsystraywidth();
	XMoveResizeWindow(dpy, m->barwin, m->wx + sp, m->by + vp, w, bh);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
    XWindowChanges wc;

    c->oldx = c->x; c->x = wc.x = x;
    c->oldy = c->y; c->y = wc.y = y;
    c->oldw = c->w; c->w = wc.width = w;
    c->oldh = c->h; c->h = wc.height = h;
	if (c->beingmoved)
		return;
    wc.border_width = c->bw;
    XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
    configure(c);
    XSync(dpy, False);
}

void
resizerequest(XEvent *e)
{
	XResizeRequestEvent *ev = &e->xresizerequest;
	Client *i;

	if ((i = wintosystrayicon(ev->window))) {
		updatesystrayicongeom(i, ev->width, ev->height);
		resizebarwin(selmon);
		updatesystray();
	}
}

void
resizemouse(const Arg *arg)
{
    int ocx, ocy, nw, nh;
    int ocx2, ocy2, nx, ny;
    Client *c;
    Monitor *m;
    XEvent ev;
	int horizcorner, vertcorner;
	int di;
	unsigned int dui;
	Window dummy;
    Time lasttime = 0;

    if (!(c = selmon->sel))
        return;
    if (c->isfullscreen && c->fakefullscreen != 1) /* no support resizing fullscreen windows by mouse */
        return;
    restack(selmon);
    ocx = c->x;
    ocy = c->y;
	ocx2 = c->x + c->w;
	ocy2 = c->y + c->h;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
        None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
        return;
    if (c->isfloating || NULL == c->mon->lt[c->mon->sellt]->arrange) {
    	if (!XQueryPointer (dpy, c->win, &dummy, &dummy, &di, &di, &nx, &ny, &dui))
    		return;
    	horizcorner = nx < c->w / 2;
    	vertcorner  = ny < c->h / 2;
    	XWarpPointer (dpy, None, c->win, 0, 0, 0, 0,
    			horizcorner ? (-c->bw) : (c->w + c->bw -1),
    			vertcorner  ? (-c->bw) : (c->h + c->bw -1));
    } else {
        XWarpPointer(dpy, None, root, 0, 0, 0, 0,
            selmon->mx + (selmon->ww * selmon->mfact),
            selmon->my + (selmon->wh / 2)
        );
    }

    do {
        XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
        switch(ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 60))
                continue;
            lasttime = ev.xmotion.time;

			nx = horizcorner ? ev.xmotion.x : c->x;
			ny = vertcorner ? ev.xmotion.y : c->y;
			nw = MAX(horizcorner ? (ocx2 - nx) : (ev.xmotion.x - ocx - 2 * c->bw + 1), 1);
			nh = MAX(vertcorner ? (ocy2 - ny) : (ev.xmotion.y - ocy - 2 * c->bw + 1), 1);

            if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
                resize(c, nx, ny, nw, nh, 1);
            break;
        }
    } while (ev.type != ButtonRelease);
    
    if (c->isfloating || NULL == c->mon->lt[c->mon->sellt]->arrange) {
        XWarpPointer(dpy, None, c->win, 0, 0, 0, 0,
		    horizcorner ? (-c->bw) : (c->w + c->bw - 1),
		    vertcorner ? (-c->bw) : (c->h + c->bw - 1));
    } else {
        selmon->mfact = (double) (ev.xmotion.x_root - selmon->mx) / (double) selmon->ww;
        arrange(selmon);
        XWarpPointer(dpy, None, root, 0, 0, 0, 0,
            selmon->mx + (selmon->ww * selmon->mfact),
            selmon->my + (selmon->wh / 2)
        );
    }

    XUngrabPointer(dpy, CurrentTime);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
        sendmon(c, m);
        selmon = m;
        focus(NULL);
    }
}

void
restack(Monitor *m)
{
    Client *c;
    XEvent ev;
    XWindowChanges wc;

    drawbar(m);
    if (!m->sel)
        return;
    if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
        XRaiseWindow(dpy, m->sel->win);
    if (m->lt[m->sellt]->arrange) {
        wc.stack_mode = Below;
        wc.sibling = m->barwin;
        for (c = m->stack; c; c = c->snext)
            if (!c->isfloating && ISVISIBLE(c)) {
                XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
                wc.sibling = c->win;
            }
    }
    XSync(dpy, False);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
rotatestack(const Arg *arg)
{
	Client *c = NULL, *f;

	if (!selmon->sel)
		return;
	f = selmon->sel;
	if (arg->i > 0) {
		for (c = nexttiled(selmon->clients); c && nexttiled(c->next); c = nexttiled(c->next));
		if (c){
			detach(c);
			attach(c);
			detachstack(c);
			attachstack(c);
		}
	} else {
		if ((c = nexttiled(selmon->clients))){
			detach(c);
			enqueue(c);
			detachstack(c);
			enqueuestack(c);
		}
	}
	if (c){
		arrange(selmon);
		//unfocus(f, 1);
		focus(f);
		restack(selmon);
	}
}

void
run(void)
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
				if (handlexevent(events + i) == -1)
					return;
			} else if (event_fd == ipc_get_sock_fd()) {
				ipc_handle_socket_epoll_event(events + i);
			} else if (ipc_is_client_registered(event_fd)){
				if (ipc_handle_client_epoll_event(events + i, mons, &lastselmon, selmon,
							tags, LENGTH(tags), layouts, LENGTH(layouts)) < 0) {
					fprintf(stderr, "Error handling IPC event on fd %d\n", event_fd);
				}
			} else {
				fprintf(stderr, "Got event from unknown fd %d, ptr %p, u32 %d, u64 %lu",
						event_fd, events[i].data.ptr, events[i].data.u32,
						events[i].data.u64);
				fprintf(stderr, " with events %d\n", events[i].events);
				return;
			}
		}
	}
}

void
scan(void)
{
    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa)
            || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
                continue;
            if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
                manage(wins[i], &wa);
        }
        for (i = 0; i < num; i++) { /* now the transients */
            if (!XGetWindowAttributes(dpy, wins[i], &wa))
                continue;
            if (XGetTransientForHint(dpy, wins[i], &d1)
            && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
                manage(wins[i], &wa);
        }
        if (wins)
            XFree(wins);
    }
}

void
sendmon(Client *c, Monitor *m)
{
    if (c->mon == m)
        return;
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

void
setclientstate(Client *c, long state)
{
    long data[] = { state, None };

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
        PropModeReplace, (unsigned char *)data, 2);
}

void
setcurrentdesktop(void){
	long data[] = { 0 };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void 
setdesktopnames(void){
	XTextProperty text;

    // Added because im lazy and dont want to have to write IPC to solve a simple warning.
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
	Xutf8TextListToTextProperty(dpy, tags, TAGSLENGTH, XUTF8StringStyle, &text);
    #pragma GCC diagnostic pop
    
	XSetTextProperty(dpy, root, &text, netatom[NetDesktopNames]);
}

int
sendevent(Window w, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4)
{
    int n;
    Atom *protocols, mt;
    int exists = 0;
    XEvent ev;

	if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
		mt = wmatom[WMProtocols];
		if (XGetWMProtocols(dpy, w, &protocols, &n)) {
			while (!exists && n--)
				exists = protocols[n] == proto;
			XFree(protocols);
		}
	}
	else {
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

void
setnumdesktops(void){
	long data[] = { TAGSLENGTH };
	XChangeProperty(dpy, root, netatom[NetNumberOfDesktops], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

void
setfocus(Client *c)
{
    if (!c->neverfocus) {
        XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(dpy, root, netatom[NetActiveWindow],
            XA_WINDOW, 32, PropModeReplace,
            (unsigned char *) &(c->win), 1);
    }
    sendevent(c->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void
setfullscreen(Client *c, int fullscreen)
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
            XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
        else
            XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                PropModeReplace, (unsigned char*)0, 0);
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
        c->bw = 0;
        c->isfloating = 1;
        resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        XRaiseWindow(dpy, c->win);
    } else if (restorestate && (c->oldstate & (1 << 1))) {
        c->bw = c->oldbw;
        c->isfloating = c->oldstate = c->oldstate & 1;
        if (restorefakefullscreen || c->fakefullscreen == 3)
            c->fakefullscreen = 1;
        /* The client may have been moved to another monitor whilst in fullscreen which if tiled
         * we address by doing a full arrange of tiled clients. If the client is floating then the
         * height and width may be larger than the monitor's window area, so we cap that by
         * ensuring max / min values. */
        if (c->isfloating) {
            c->x = MAX(c->mon->wx, c->oldx);
            c->y = MAX(c->mon->wy, c->oldy);
            c->w = MIN(c->mon->ww - c->x - 2*c->bw, c->oldw);
            c->h = MIN(c->mon->wh - c->y - 2*c->bw, c->oldh);
            resizeclient(c, c->x, c->y, c->w, c->h);
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

void
layoutscroll(const Arg *arg)
{
	if (!arg || !arg->i)
		return;
	int switchto = selmon->ltcur + arg->i;
	int l = LENGTH(layouts);

	if (switchto == l)
		switchto = 0;
	else if(switchto < 0)
		switchto = l - 1;

	selmon->ltcur = switchto;
	Arg arg2 = {.v= &layouts[switchto] };
	setlayout(&arg2);

}

void
setsticky(Client *c, int sticky)
{

    if(sticky && !c->issticky) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                PropModeReplace, (unsigned char *) &netatom[NetWMSticky], 1);
        c->issticky = 1;
    } else if(!sticky && c->issticky){
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
                PropModeReplace, (unsigned char *)0, 0);
        c->issticky = 0;
        arrange(c->mon);
    }
}

void
setlayout(const Arg *arg)
{
    if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
    if (arg && arg->v)
        selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] = (Layout *)arg->v;
    strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
    if (selmon->sel)
        arrange(selmon);
    else
        drawbar(selmon);
}

void
setlayoutsafe(const Arg *arg)
{
	const Layout *ltptr = (Layout *)arg->v;
	if (ltptr == 0)
			setlayout(arg);
	for (int i = 0; i < LENGTH(layouts); i++) {
		if (ltptr == &layouts[i])
			setlayout(arg);
	}
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
    float f;

    if (!arg || !selmon->lt[selmon->sellt]->arrange)
        return;
    f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
    if (f < 0.05 || f > 0.95)
        return;
    selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
    arrange(selmon);
}

void
setup(void)
{
    int i;
    XSetWindowAttributes wa;
    Atom utf8string;

    /* clean up any zombies immediately */
    sigchld(0);
	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);

    /* init screen */
    screen = DefaultScreen(dpy);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);
    root = RootWindow(dpy, screen);
    drw = drw_create(dpy, screen, root, sw, sh);
    if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
        die("no fonts could be loaded.");
    lrpad = drw->fonts->h + horizpadbar;
    bh = drw->fonts->h + vertpadbar;
	sp = sidepad;
	vp = (topbar == 1) ? vertpad : - vertpad;
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
    netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	netatom[NetDesktopViewport] = XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", False);
	netatom[NetNumberOfDesktops] = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);
	netatom[NetCurrentDesktop] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);
	netatom[NetDesktopNames] = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	xatom[Manager] = XInternAtom(dpy, "MANAGER", False);
	xatom[Xembed] = XInternAtom(dpy, "_XEMBED", False);
	xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);
    netatom[NetClientInfo] = XInternAtom(dpy, "_NET_CLIENT_INFO", False);
    motifatom = XInternAtom(dpy, "_MOTIF_WM_HINTS", False);
    /* init cursors */
    cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
    cursor[CurResize] = drw_cur_create(drw, XC_sizing);
    cursor[CurMove] = drw_cur_create(drw, XC_fleur);
    cursor[CurSwal] = drw_cur_create(drw, XC_bottom_side);
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
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
        PropModeReplace, (unsigned char *) &wmcheckwin, 1);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
        PropModeReplace, (unsigned char *) "dwm", 3);
    XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
        PropModeReplace, (unsigned char *) &wmcheckwin, 1);
    /* EWMH support per view */
    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
        PropModeReplace, (unsigned char *) netatom, NetLast);
	setnumdesktops();
	setcurrentdesktop();
	setdesktopnames();
	setviewport();
    XDeleteProperty(dpy, root, netatom[NetClientList]);
    XDeleteProperty(dpy, root, netatom[NetClientInfo]);
    /* select events */
    wa.cursor = cursor[CurNormal]->cursor;
    wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
        |ButtonPressMask|PointerMotionMask|EnterWindowMask
        |LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
    XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);
    grabkeys();
    focus(NULL);
    setupepoll();
}

void
setupepoll(void)
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

void
setviewport(void){
	long data[] = { 0, 0 };
	XChangeProperty(dpy, root, netatom[NetDesktopViewport], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 2);
}

void
seturgent(Client *c, int urg)
{
    XWMHints *wmh;

    c->isurgent = urg;
    if (!(wmh = XGetWMHints(dpy, c->win)))
        return;
    wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
    XSetWMHints(dpy, c->win, wmh);
    XFree(wmh);
}

void
showhide(Client *c)
{
    if (!c)
        return;
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

void
sigchld(int unused)
{
    pid_t pid;
    if (signal(SIGCHLD, sigchld) == SIG_ERR) {
        die("can't install SIGCHLD handler:");
    }
	while (0 < (pid = waitpid(-1, NULL, WNOHANG))) {
		pid_t *p, *startandrestart_lim, *startonce_lim;

		if (!(p = startandrestart_pids) || !(p = startonce_pids))
            continue;

        startandrestart_lim = &p[startandrestart_len];
        startonce_lim = &p[startonce_len];

		for (; p < startandrestart_lim; p++) {
			if (*p == pid) {
				*p = -1;
				break;
			}
		}

        for (; p < startonce_lim; p++) {
			if (*p == pid) {
				*p = -1;
				break;
			}
		}

	}
}

void
sighup(int unused)
{
	Arg a = {.i = 1};
	quit(&a);
}

void
sigterm(int unused)
{
	Arg a = {.i = 0};
	quit(&a);
}

void
spawn(const Arg *arg)
{
    selmon->tagset[selmon->seltags] &= ~scratchtag;
    if (fork() == 0) {
        if (dpy)
            close(ConnectionNumber(dpy));
        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
    }
}

/*
 * Perform immediate swallow of client 'swee' by client 'swer'. 'manage' shall
 * be set if swal() is called from swalmanage(). 'swer' and 'swee' must be
 * regular or swallowee, but not swallower.
 */
void
swal(Client *swer, Client *swee, int manage)
{
	Client *c, **pc;
	int sweefocused = selmon->sel == swee;

	/* No self-swallowing! */
	if (swer == swee)
		return;

	/* Remove any swallows registered for the swer. Asking a swallower to
	 * swallow another window is ambiguous and is thus avoided altogether. In
	 * contrast, a swallowee can swallow in a well-defined manner by attaching
	 * to the head of the swallow chain. */
	if (!manage)
		swalunreg(swer);

	/* Disable fullscreen prior to swallow. Swallows involving fullscreen
	 * windows produces quirky artefacts such as fullscreen terminals or tiled
	 * pseudo-fullscreen windows. */
	setfullscreen(swer, 0);
	setfullscreen(swee, 0);

	/* Swap swallowee into client and focus lists. Keeps current focus unless
	 * the swer (which gets unmapped) is focused in which case the swee will
	 * receive focus. */
	detach(swee);
	for (pc = &swer->mon->clients; *pc && *pc != swer; pc = &(*pc)->next);
	*pc = swee;
	swee->next = swer->next;
	detachstack(swee);
	for (pc = &swer->mon->stack; *pc && *pc != swer; pc = &(*pc)->snext);
	*pc = swee;
	swee->snext = swer->snext;
	swee->mon = swer->mon;
	if (sweefocused) {
		detachstack(swee);
		attachstack(swee);
		selmon = swer->mon;
	}
	swee->tags = swer->tags;
	swee->isfloating = swer->isfloating;
	for (c = swee; c->swer; c = c->swer);
	c->swer = swer;

	/* Configure geometry params obtained from patches (e.g. cfacts) here. */
	// swee->cfact = swer->cfact;

	/* ICCCM 4.1.3.1 */
	setclientstate(swer, WithdrawnState);
	if (manage)
		setclientstate(swee, NormalState);

	if (swee->isfloating || !swee->mon->lt[swee->mon->sellt]->arrange)
		XRaiseWindow(dpy, swee->win);
	//resize(swee, swer->x, swer->y, swer->w, swer->h, 0);
	focus(NULL);
	arrange(NULL);
	if (manage)
		XMapWindow(dpy, swee->win);
	XUnmapWindow(dpy, swer->win);
	restack(swer->mon);
}

/*
 * Register a future swallow with swallower 'c'. 'class', 'inst' and 'title'
 * shall point null-terminated strings and must not be NULL. If an already
 * existing swallow instance targets 'c' its filters are updated and no new
 * swallow instance is created. 'c' may be ClientRegular or ClientSwallowee.
 * Complement to swalrm().
 */
void
swalreg(Client *c, const char *class, const char *inst, const char *title)
{
	Swallow *s;

	if (!c)
		return;

	/* Update existing swallow */
	for (s = swallows; s; s = s->next) {
		if (s->client == c) {
			strncpy(s->class, class, sizeof(s->class) - 1);
			strncpy(s->inst, inst, sizeof(s->inst) - 1);
			strncpy(s->title, title, sizeof(s->title) - 1);
			s->decay = swaldecay;

			/* Only one swallow per client. May return after first hit. */
			return;
		}
	}

	s = ecalloc(1, sizeof(Swallow));
	s->decay = swaldecay;
	s->client = c;
	strncpy(s->class, class, sizeof(s->class) - 1);
	strncpy(s->inst, inst, sizeof(s->inst) - 1);
	strncpy(s->title, title, sizeof(s->title) - 1);

	s->next = swallows;
	swallows = s;
}

/*
+ * Decrease decay counter of all registered swallows by 'decayby' and remove any
 * swallow instances whose counter is less than or equal to zero.
 */
void
swaldecayby(int decayby)
{
	Swallow *s, *t;

	for (s = swallows; s; s = t) {
		s->decay -= decayby;
		t = s->next;
		if (s->decay <= 0)
			swalrm(s);
	}
}

/*
 * Window configuration and client setup for new windows which are to be
 * swallowed immediately. Pendant to manage() for such windows.
 */
void
swalmanage(Swallow *s, Window w, XWindowAttributes *wa)
{
	Client *swee, *swer;
	XWindowChanges wc;

	swer = s->client;
	swalrm(s);

	/* Perform bare minimum setup of a client for window 'w' such that swal()
	 * may be used to perform the swallow. The following lines are basically a
	 * minimal implementation of manage() with a few chunks delegated to
	 * swal(). */
	swee = ecalloc(1, sizeof(Client));
	swee->win = w;
	swee->mon = swer->mon;
	swee->oldbw = wa->border_width;
	swee->bw = borderpx;
	attach(swee);
	attachstack(swee);
	updatetitle(swee);
	updatesizehints(swee);
	XSelectInput(dpy, swee->win, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	wc.border_width = swee->bw;
	XConfigureWindow(dpy, swee->win, CWBorderWidth, &wc);
	grabbuttons(swee, 0);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
		(unsigned char *) &(swee->win), 1);

	swal(swer, swee, 1);
}

/*
+ * Return swallow instance which targets window 'w' as determined by its class
 * name, instance name and window title. Returns NULL if none is found. Pendant
 * to wintoclient().
 */
Swallow *
swalmatch(Window w)
{
	XClassHint ch = { NULL, NULL };
	Swallow *s = NULL;
	char title[sizeof(s->title)];

	XGetClassHint(dpy, w, &ch);
	if (!gettextprop(w, netatom[NetWMName], title, sizeof(title)))
		gettextprop(w, XA_WM_NAME, title, sizeof(title));

	for (s = swallows; s; s = s->next) {
		if ((!ch.res_class || strstr(ch.res_class, s->class))
			&& (!ch.res_name || strstr(ch.res_name, s->inst))
			&& (title[0] == '\0' || strstr(title, s->title)))
			break;
	}

	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
	return s;
}

/*
 * Interactive drag-and-drop swallow.
 */
void
swalmouse(const Arg *arg)
{
	Client *swer, *swee;
	XEvent ev;

	if (!(swee = selmon->sel))
		return;

	if (XGrabPointer(dpy, root, False, ButtonPressMask|ButtonReleaseMask, GrabModeAsync,
		GrabModeAsync, None, cursor[CurSwal]->cursor, CurrentTime) != GrabSuccess)
		return;

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest: /* fallthrough */
		case Expose: /* fallthrough */
		case MapRequest:
			handler[ev.type](&ev);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);

	if ((swer = wintoclient(ev.xbutton.subwindow))
		&& swer != swee)
		swal(swer, swee, 0);

	/* Remove accumulated pending EnterWindow events caused by the mouse
	 * movements. */
	XCheckMaskEvent(dpy, EnterWindowMask, &ev);
}

/*
 * Delete swallow instance swallows and free its resources. Complement to
 * swalreg(). If NULL is passed all registered swallows are deleted.
 */
void
swalrm(Swallow *s)
{
	Swallow *t, **ps;

	if (s) {
		for (ps = &swallows; *ps && *ps != s; ps = &(*ps)->next);
		*ps = s->next;
		free(s);
	}
	else {
		for(s = swallows; s; s = t) {
			t = s->next;
			free(s);
		}
		swallows = NULL;
	}
}

/*
 * Removes swallow instance targeting 'c' if it exists. Complement to swalreg().
 */
void
swalunreg(Client *c) { Swallow *s;

	for (s = swallows; s; s = s->next) {
		if (c == s->client) {
			swalrm(s);
			/* Max. 1 registered swallow per client. No need to continue. */
			break;
		}
	}
}

/*
 * Stop an active swallow of swallowed client 'swee' and remap the swallower.
 * If 'swee' is a swallower itself 'root' must point the root client of the
 * swallow chain containing 'swee'.
 */
void
swalstop(Client *swee, Client *root)
{
	Client *swer;

	if (!swee || !(swer = swee->swer))
		return;

	swee->swer = NULL;
	root = root ? root : swee;
	swer->mon = root->mon;
	swer->tags = root->tags;
	swer->next = root->next;
	root->next = swer;
	swer->snext = root->snext;
	root->snext = swer;
	swer->isfloating = swee->isfloating;

	/* Configure geometry params obtained from patches (e.g. cfacts) here. */
	// swer->cfact = 1.0;

	/* If swer is not in tiling mode reuse swee's geometry. */
	if (swer->isfloating || !root->mon->lt[root->mon->sellt]->arrange) {
		XRaiseWindow(dpy, swer->win);
		resize(swer, swee->x, swee->y, swee->w, swee->h, 0);
	}

	/* Override swer's border scheme which may be using SchemeSel. */
	XSetWindowBorder(dpy, swer->win, scheme[SchemeNorm][ColBorder].pixel);

	/* ICCCM 4.1.3.1 */
	setclientstate(swer, NormalState);

	XMapWindow(dpy, swer->win);
	focus(NULL);
	arrange(swer->mon);
}

/*
 * Stop active swallow for currently selected client.
 */
void
swalstopsel(const Arg *unused)
{
	if (selmon->sel)
		swalstop(selmon->sel, NULL);
}

void
setclienttagprop(Client *c)
{
    long data[] = { (long) c->tags, (long) c->mon->num };
    XChangeProperty(dpy, c->win, netatom[NetClientInfo], XA_CARDINAL, 32,
            PropModeReplace, (unsigned char *) data, 2);
}

void
tag(const Arg *arg)
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

void
tagmon(const Arg *arg)
{
    Client *c = selmon->sel;
    if (!c || !mons->next)
        return;
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

void
togglebar(const Arg *arg)
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
			if (!selmon->topbar)
				wc.y = selmon->mh - bh + vp;
		}
		XConfigureWindow(dpy, systray->win, CWY, &wc);
	}
    arrange(selmon);
}

void
togglealttag(const Arg *arg)
{
	selmon->alttag = !selmon->alttag;
	drawbar(selmon);
}

void
togglefakefullscreen(const Arg *arg)
{
    Client *c = selmon->sel;
    if (!c)
        return;

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

void
togglefloating(const Arg *arg)
{
    Client *c = selmon->sel;
    if (!c)
        return;
    if (c->isfullscreen && c->fakefullscreen != 1) /* no support for fullscreen windows */
        return;
    c->isfloating = !c->isfloating || c->isfixed;
	if (c->isfloating) {
        XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColFloat].pixel);
		c->bw = fborderpx;
		configure(c);
        int borderdiff = (fborderpx - borderpx) * 2;
        resize(c, c->x, c->y, c->w - borderdiff, c->h - borderdiff, 0);
	} else {
        XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		c->bw = borderpx;
		configure(c);
	}
    arrange(c->mon);
}

void
centerfloating(const Arg *arg)
{
    Client *c = selmon->sel;
    int nx, ny;

    if (c->isfloating) {
        nx = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
        ny = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;

        resize(c, nx, ny, c->w, c->h, 0);
        XWarpPointer(dpy, None, c->mon->sel->win, 0, 0, 0, 0, c->mon->sel->w/2, c->mon->sel->h/2);
    }
}

void
togglescratch(const Arg *arg)
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
		}
	} else
		spawn(arg);
}

void
togglesticky(const Arg *arg)
{
	if (!selmon->sel)
		return;
    setsticky(selmon->sel, !selmon->sel->issticky);
	arrange(selmon);
}

void
togglefullscreen(const Arg *arg)
{
    Client *c = selmon->sel;
    if (!c)
        return;

    if (c->fakefullscreen == 1) { // fake fullscreen --> fullscreen
        c->fakefullscreen = 2;
        setfullscreen(c, 1);
    } else
        setfullscreen(c, !c->isfullscreen);
}

void
toggletag(const Arg *arg)
{
    unsigned int newtags;

    if (!selmon->sel)
        return;
    newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
    if (newtags) {
        selmon->sel->tags = newtags;
        setclienttagprop(selmon->sel);
        focus(NULL);
        arrange(selmon);
    }
    updatecurrentdesktop();
}

void
toggleview(const Arg *arg)
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
            for (i = 0; !(newtagset & 1 << i); i++) ;
            selmon->pertag->curtag = i + 1;
        }

        /* apply settings for this view */
        selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
        selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
        selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
        selmon->lt[selmon->sellt] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt];
        selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];

        if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
            togglebar(NULL);
        focus(NULL);
        arrange(selmon);
    }
    updatecurrentdesktop();
}

void
freeicon(Client *c)
{
	if (c->icon) {
		XRenderFreePicture(dpy, c->icon);
		c->icon = None;
	}
}

void
warp(const Client *c)
{
    int x, y;

    if (!c) {
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, selmon->wx + selmon->ww/2, selmon->wy + selmon->wh/2);
        return;
    }

    if (!getrootptr(&x, &y) ||
        (x > c->x - c->bw &&
         y > c->y - c->bw &&
         x < c->x + c->w + c->bw*2 &&
         y < c->y + c->h + c->bw*2) ||
        (y > c->mon->by && y < c->mon->by + bh) ||
        (c->mon->topbar && !y))
        return;

    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
}

void
unfocus(Client *c, int setfocus)
{
    if (!c)
        return;
    grabbuttons(c, 0);
    lastfocused = c;
    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

void
unmanage(Client *c, int destroyed)
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
	if (lastfocused == c)
		lastfocused = NULL;
    free(c);
    focus(NULL);
    updateclientlist();
    arrange(m);
    if (m == selmon && m->sel)
        XWarpPointer(dpy, None, m->sel->win, 0, 0, 0, 0,
                     m->sel->w/2, m->sel->h/2);
}

void
unmapnotify(XEvent *e)
{
    Client *c;
    XUnmapEvent *ev = &e->xunmap;
    int type;

	type = wintoclient2(ev->window, &c, NULL);
	if (type && ev->send_event) {
		setclientstate(c, WithdrawnState);
		return;
	}
	switch (type) {
	case ClientRegular:
		unmanage(c, 0);
		break;
	case ClientSwallowee:
		swalstop(c, NULL);
		unmanage(c, 0);
		break;
	case ClientSwallower:
		/* Swallowers are never mapped. Nothing to do. */
		break;
    }
	if ((c = wintosystrayicon(ev->window))) {
		/* KLUDGE! sometimes icons occasionally unmap their windows, but do
		 * _not_ destroy them. We map those windows back */
		XMapRaised(dpy, c->win);
		updatesystray();
	}
}

void
updatebars(void)
{
    unsigned int w;
    Monitor *m;
    XSetWindowAttributes wa = {
        .override_redirect = True,
        .background_pixmap = ParentRelative,
        .event_mask = ButtonPressMask|ExposureMask
    };
    XClassHint ch = {"dwm", "dwm"};
    for (m = mons; m; m = m->next) {
        if (m->barwin)
            continue;
        w = m->ww - 2 * sp;
		if (showsystray && m == systraytomon(m))
			w -= getsystraywidth();
		m->barwin = XCreateWindow(dpy, root, m->wx + sp, m->by + vp, w, bh, 0, DefaultDepth(dpy, screen),
                CopyFromParent, DefaultVisual(dpy, screen),
                CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);
        XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
		if (showsystray && m == systraytomon(m))
			XMapRaised(dpy, systray->win);
        XMapRaised(dpy, m->barwin);
        XSetClassHint(dpy, m->barwin, &ch);
    }
}

void
updatebarpos(Monitor *m)
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

void
updateclientlist()
{
    Client *c, *d;
    Monitor *m;

    XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (m = mons; m; m = m->next) {
		for (c = m->clients; c; c = c->next) {
			for (d = c; d; d = d->swer) {
				XChangeProperty(dpy, root, netatom[NetClientList],
					XA_WINDOW, 32, PropModeAppend,
					(unsigned char *) &(c->win), 1);
			}
		}
	}
}

void 
updatecurrentdesktop(void){
	long rawdata[] = { selmon->tagset[selmon->seltags] };
	int i=0;
	while(*rawdata >> (i+1)){
		i++;
	}
	long data[] = { i };
	XChangeProperty(dpy, root, netatom[NetCurrentDesktop], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, 1);
}

int
updategeom(void)
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
            if (i >= n
            || unique[i].x_org != m->mx || unique[i].y_org != m->my
            || unique[i].width != m->mw || unique[i].height != m->mh)
            {
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
            if (m == selmon)
                selmon = mons;
            cleanupmon(m);
        }
        free(unique);
    } else
#endif /* XINERAMA */
    { /* default monitor setup */
        if (!mons)
            mons = createmon();
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

void
updatemotifhints(Client *c)
{
    Atom real;
    int format;
    unsigned char *p = NULL;
    unsigned long n, extra;
    unsigned long *motif;
    int width, height;

    if (!decorhints)
        return;

    if (XGetWindowProperty(dpy, c->win, motifatom, 0L, 5L, False, motifatom,
                           &real, &format, &n, &extra, &p) == Success && p != NULL) {
        motif = (unsigned long*)p;
        if (motif[MWM_HINTS_FLAGS_FIELD] & MWM_HINTS_DECORATIONS) {
            width = WIDTH(c);
            height = HEIGHT(c);

            if (motif[MWM_HINTS_DECORATIONS_FIELD] & MWM_DECOR_ALL ||
                motif[MWM_HINTS_DECORATIONS_FIELD] & MWM_DECOR_BORDER ||
                motif[MWM_HINTS_DECORATIONS_FIELD] & MWM_DECOR_TITLE)
                if (c->isfloating) {
                    c->bw = c->oldbw = fborderpx;
                } else {
                    c->bw = c->oldbw = borderpx;
                }
            else
                c->bw = c->oldbw = 0;

            resize(c, c->x, c->y, width - (2*c->bw), height - (2*c->bw), 0);
        }
        XFree(p);
    }
}

void
updatenumlockmask(void)
{
    unsigned int i, j;
    XModifierKeymap *modmap;

    numlockmask = 0;
    modmap = XGetModifierMapping(dpy);
    for (i = 0; i < 8; i++)
        for (j = 0; j < modmap->max_keypermod; j++)
            if (modmap->modifiermap[i * modmap->max_keypermod + j]
                == XKeysymToKeycode(dpy, XK_Num_Lock))
                numlockmask = (1 << i);
    XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c)
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

void
updatestatus(void)
{
    if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)) && showstatus)
        strcpy(stext, "dwm-"VERSION);
    drawbar(selmon);
    updatesystray();
}

void
updaterules(Client *c)
{
    //applyrules(c)
	const char *class, *instance;
	unsigned int i;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;
    char found_rule = 0;

	for (i = 0; i < LENGTH(rules); i++) {
		r = &rules[i];
		if ((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->instance || strstr(instance, r->instance)))
		{
			c->isfloating = r->isfloating;

            if(!found_rule)
            {
                c->tags=0;
			    found_rule=1;
            }    
            c->tags |= r->tags;
            for (m = mons; m && m->num != r->monitor; m = m->next){};  /* Not sure where this is from, havent fixed, pretty sure its useless, will maybe get rid of it later */
			if (m)
				c->mon = m;
		}
	}
	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);
	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];

    // end apply rules
	if (c->isfloating)
		resize(c, c->x, c->y,
			c->w, c->h, 0);
	arrange(c->mon);
}

void
updatesystrayicongeom(Client *i, int w, int h)
{
	if (i) {
		i->h = bh;
		if (w == h)
			i->w = bh;
		else if (h == bh)
			i->w = w;
		else
			i->w = (int) ((float)bh * ((float)w / (float)h));
		applysizehints(i, &(i->x), &(i->y), &(i->w), &(i->h), False);
		/* force icons into the systray dimensions if they don't want to */
		if (i->h > bh) {
			if (i->w == i->h)
				i->w = bh;
			else
				i->w = (int) ((float)bh * ((float)i->w / (float)i->h));
			i->h = bh;
		}
	}
}

void
updatesystrayiconstate(Client *i, XPropertyEvent *ev)
{
	long flags;
	int code = 0;

	if (!showsystray || !i || ev->atom != xatom[XembedInfo] ||
			!(flags = getatomprop(i, xatom[XembedInfo])))
		return;

	if (flags & XEMBED_MAPPED && !i->tags) {
		i->tags = 1;
		code = XEMBED_WINDOW_ACTIVATE;
		XMapRaised(dpy, i->win);
		setclientstate(i, NormalState);
	}
	else if (!(flags & XEMBED_MAPPED) && i->tags) {
		i->tags = 0;
		code = XEMBED_WINDOW_DEACTIVATE;
		XUnmapWindow(dpy, i->win);
		setclientstate(i, WithdrawnState);
	}
	else
		return;
	sendevent(i->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0,
			systray->win, XEMBED_EMBEDDED_VERSION);
}

void
updatesystray(void)
{
	XSetWindowAttributes wa;
	XWindowChanges wc;
	Client *i;
	Monitor *m = systraytomon(NULL);
	unsigned int x = m->mx + m->mw;
	unsigned int sw = TEXTW(stext) - lrpad + systrayspacing;
	unsigned int w = 1;

	if (!showsystray)
		return;
	if (systrayonleft)
		x -= sw + lrpad / 2;
	if (!systray) {
		/* init systray */
		if (!(systray = (Systray *)calloc(1, sizeof(Systray))))
			die("fatal: could not malloc() %u bytes\n", sizeof(Systray));
		systray->win = XCreateSimpleWindow(dpy, root, x - sp, m->by + vp, w, bh, 0, 0, scheme[SchemeSel][ColBg].pixel);
		wa.event_mask        = ButtonPressMask | ExposureMask;
		wa.override_redirect = True;
		wa.background_pixel  = scheme[SchemeNorm][ColBg].pixel;
		XSelectInput(dpy, systray->win, SubstructureNotifyMask);
		XChangeProperty(dpy, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *)&netatom[NetSystemTrayOrientationHorz], 1);
		XChangeWindowAttributes(dpy, systray->win, CWEventMask|CWOverrideRedirect|CWBackPixel, &wa);
		XMapRaised(dpy, systray->win);
		XSetSelectionOwner(dpy, netatom[NetSystemTray], systray->win, CurrentTime);
		if (XGetSelectionOwner(dpy, netatom[NetSystemTray]) == systray->win) {
			sendevent(root, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray], systray->win, 0, 0);
			XSync(dpy, False);
		}
		else {
			fprintf(stderr, "dwm: unable to obtain system tray.\n");
			free(systray);
			systray = NULL;
			return;
		}
	}
	for (w = 0, i = systray->icons; i; i = i->next) {
		/* make sure the background color stays the same */
		wa.background_pixel  = scheme[SchemeNorm][ColBg].pixel;
		XChangeWindowAttributes(dpy, i->win, CWBackPixel, &wa);
		XMapRaised(dpy, i->win);
		w += systrayspacing;
		i->x = w;
		XMoveResizeWindow(dpy, i->win, i->x, 0, i->w, i->h);
		w += i->w;
		if (i->mon != m)
			i->mon = m;
	}
	w = w ? w + systrayspacing : 1;
	x -= w;
	XMoveResizeWindow(dpy, systray->win, x - sp, m->by + vp, w, bh);
	wc.x = x - sp; wc.y = m->by + vp; wc.width = w; wc.height = bh;
	wc.stack_mode = Above; wc.sibling = m->barwin;
	XConfigureWindow(dpy, systray->win, CWX|CWY|CWWidth|CWHeight|CWSibling|CWStackMode, &wc);
	XMapWindow(dpy, systray->win);
	XMapSubwindows(dpy, systray->win);
	/* redraw background */
	XSetForeground(dpy, drw->gc, scheme[SchemeNorm][ColBg].pixel);
	XFillRectangle(dpy, systray->win, drw->gc, 0, 0, w, bh);
	XSync(dpy, False);
}

void
updatetitle(Client *c)
{
	char oldname[sizeof(c->name)];
	strcpy(oldname, c->name);

    if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
        gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
    if (c->name[0] == '\0'){ /* hack to mark broken clients */
        strcpy(c->name, broken);
    }    
	for (Monitor *m = mons; m; m = m->next) {
		if (m->sel == c && strcmp(oldname, c->name) != 0)
			ipc_focused_title_change_event(m->num, c->win, oldname, c->name);
	}
}

void
updateicon(Client *c)
{
	freeicon(c);
	c->icon = geticonprop(c->win, &c->icw, &c->ich);
}

void
updatewindowtype(Client *c)
{
    Atom state = getatomprop(c, netatom[NetWMState]);
    Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

    if (state == netatom[NetWMFullscreen])
        setfullscreen(c, 1);
    if (state == netatom[NetWMSticky]) {
        setsticky(c, 1);
    }
    if (wtype == netatom[NetWMWindowTypeDialog])
        c->isfloating = 1;
}

void
updatewmhints(Client *c)
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

void
view(const Arg *arg)
{
    int i;
    unsigned int tmptag;
    if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
        return;
    selmon->seltags ^= 1; /* toggle sel tagset */
    if (arg->ui & TAGMASK) {
        selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
        selmon->pertag->prevtag = selmon->pertag->curtag;

        if (arg->ui == ~0)
            selmon->pertag->curtag = 0;
        else {
            for (i = 0; !(arg->ui & 1 << i); i++) ;
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
    selmon->lt[selmon->sellt^1] = selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt^1];

    if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
        togglebar(NULL);
    focus(NULL);
    arrange(selmon);
    updatecurrentdesktop();
}

void
viewall(const Arg *arg)
{
	Monitor *m;

	for (m = mons; m; m = m->next) {
		m->tagset[m->seltags] = arg->ui;
		arrange(m);
	}
	focus(NULL);
}

Client *
wintoclient(Window w)
{
    Client *c;
    Monitor *m;

    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            if (c->win == w)
                return c;
    return NULL;
}

/*
 * Writes client managing window 'w' into 'pc' and returns type of client. If
 * no client is found NULL is written to 'pc' and zero is returned. If a client
 * is found and is a swallower (ClientSwallower) and proot is not NULL the root
 * client of the swallow chain is written to 'proot'.
 */
int
wintoclient2(Window w, Client **pc, Client **proot)
{
	Monitor *m;
	Client *c, *d;

	for (m = mons; m; m = m->next) {
		for (c = m->clients; c; c = c->next) {
			if (c->win == w) {
				*pc = c;
				if (c->swer)
					return ClientSwallowee;
				else
					return ClientRegular;
			}
			else {
				for (d = c->swer; d; d = d->swer) {
					if (d->win == w) {
						if (proot)
							*proot = c;
						*pc = d;
						return ClientSwallower;
					}
				}
			}
		}
	}
	*pc = NULL;
	return 0;
}

Client *
wintosystrayicon(Window w) {
	Client *i = NULL;

	if (!showsystray || !w)
		return i;
	for (i = systray->icons; i && i->win != w; i = i->next) ;
	return i;
}

Monitor *
wintomon(Window w)
{
    int x, y;
    Client *c;
    Monitor *m;

    if (w == root && getrootptr(&x, &y))
        return recttomon(x, y, 1, 1);
    for (m = mons; m; m = m->next)
        if (w == m->barwin)
            return m;
    if ((c = wintoclient(w)))
        return c->mon;
    return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int
xerror(Display *dpy, XErrorEvent *ee)
{
    if (ee->error_code == BadWindow
    || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
    || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
    || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
    || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
    || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
    || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
        return 0;
    fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
        ee->request_code, ee->error_code);
    return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee)
{
    return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
    die("dwm: another window manager is already running");
    return -1;
}

Monitor *
systraytomon(Monitor *m) {
	Monitor *t;
	int i, n;
	if(!systraypinning) {
		if(!m)
			return selmon;
		return m == selmon ? m : NULL;
	}
	for(n = 1, t = mons; t && t->next; n++, t = t->next) ;
	for(i = 1, t = mons; t && t->next && i < systraypinning; i++, t = t->next) ;
	if(systraypinningfailfirst && n < systraypinning)
		return mons;
	return t;
}

void
zoom(const Arg *arg)
{
    Client *c = selmon->sel;

    if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating)
        return;
    if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next)))
        return;
    pop(c);
}

int
main(int argc, char *argv[])
{
    if (argc == 2 && !strcmp("-v", argv[1]))
        die("dwm-"VERSION);
    else if (argc == 2 && !strcmp("--first-run", argv[1]))
        autostart_exec(1);
    else if (argc != 1)
        die("usage: dwm [-v] or [--first-run]");
    else
        autostart_exec(0);

    if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
        fputs("warning: no locale support\n", stderr);
    if (!(dpy = XOpenDisplay(NULL)))
        die("dwm: cannot open display");
    checkotherwm();
    setup();
#ifdef __OpenBSD__
    if (pledge("stdio rpath proc exec", NULL) == -1)
        die("pledge");
#endif /* __OpenBSD__ */
    scan();
    run();
    char *dwm_empty_start[] = {"dwm", NULL};
    if(restart) execvp(argv[0], dwm_empty_start);
    cleanup();
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}

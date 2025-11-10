#ifndef SYSTRAY_H_
#define SYSTRAY_H_

#include <X11/X.h>
#include <X11/Xlib.h>
#include "client.h"
#include "monitor.h"

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_WINDOW_DEACTIVATE 2
#define XEMBED_REQUEST_FOCUS 3
#define XEMBED_FOCUS_IN 4
#define XEMBED_FOCUS_OUT 5
#define XEMBED_FOCUS_NEXT 6
#define XEMBED_FOCUS_PREV 7
/* 8-9 were are for XEMBED_GRAB_KEY/XEMBED_UNGRAB_KEY */
#define XEMBED_MODALITY_ON 10
#define XEMBED_MODALITY_OFF 11
#define XEMBED_REGISTER_ACCELERATOR 12
#define XEMBED_UNREGISTER_ACCELERATOR 13
#define XEMBED_ACTIVATE_ACCELERATOR 14

/* Details for  XEMBED_FOCUS_IN: */
#define XEMBED_FOCUS_CURRENT 0
#define XEMBED_FOCUS_FIRST 1
#define XEMBED_FOCUS_LAST 2

/* XEMBED misc */
#define XEMBED_MAPPED ( 1 << 0 )
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define XEMBED_EMBEDDED_VERSION ( ( VERSION_MAJOR << 16 ) | VERSION_MINOR )

typedef struct Systray {
        Window window;
        Client_t *icons;
} Systray_t;

extern unsigned int get_systray_width( void );
extern void remove_systray_icon( Client_t *icon );
extern Monitor_t *systray_to_monitor( Monitor_t *monitor );
extern void update_systray( void );
extern void update_systray_icon_geometry( Client_t *icon, int width, int height );
extern void update_systray_icon_state( Client_t *icon, const XPropertyEvent *event );
extern Client_t *window_to_systray_icon( Window window );

extern Systray_t *g_systray;

#endif /* SYSTRAY_H_ */

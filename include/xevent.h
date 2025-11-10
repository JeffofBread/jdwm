#ifndef XEVENT_H_
#define XEVENT_H_

#include <X11/X.h>
#include <X11/Xlib.h>

extern void button_press( XEvent *event );
extern void client_message( XEvent *event );
extern void configure_notify( XEvent *event );
extern void configure_request( XEvent *event );
extern void destroy_notify( XEvent *event );
extern void enter_notify( XEvent *event );
extern void expose( XEvent *event );
extern void focusin( XEvent *event );
extern void keypress( XEvent *event );
extern void mapping_notify( XEvent *event );
extern void map_request( XEvent *event );
extern void motion_notify( XEvent *event );
extern void property_notify( XEvent *event );
extern void resize_request( XEvent *event );
extern int send_event( Window window, Atom protocol, int mask, long data_0, long data_1, long data_2, long data_3, long data_4 );
extern void unmap_notify( XEvent *event );

extern void ( *handler[ LASTEvent ] )( XEvent * );

#endif /* XEVENT_H_ */

#ifndef XUTIL_H_
#define XUTIL_H_

#include <stdbool.h>
#include <stdint.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/render.h>
#include "client.h"

#define BUTTONMASK ( ButtonPressMask | ButtonReleaseMask )
#define MOUSEMASK ( BUTTONMASK | PointerMotionMask )

extern Atom get_atom_properties( const Client_t *client, Atom property );
extern Picture get_icon_properties( Window window, unsigned int *icon_width, unsigned int *icon_height );
extern int get_root_pointer( int *x, int *y );
extern long get_state( Window window );
extern int get_text_properties( Window window, Atom atom, char *text, unsigned int size );
extern void grab_buttons( const Client_t *client, bool focused );
extern void grab_keys( void );
extern uint32_t pre_alpha( uint32_t p );
extern void update_num_lock_mask( void );
extern int x_error( Display *display, XErrorEvent *error );
extern int x_error_dummy( Display *display, XErrorEvent *error );
extern int x_error_start( Display *display, XErrorEvent *error );

extern unsigned int g_num_lock_mask;
extern int ( *xerrorxlib )( Display *, XErrorEvent * );

#endif /* XUTIL_H_ */

#include "xutil.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/render.h>
#include "atoms.h"
#include "client.h"
#include "common.h"
#include "core.h"
#include "draw.h"
#include "logger.h"
#include "parser.h"

unsigned int g_num_lock_mask = 0;
int ( *xerrorxlib )( Display *, XErrorEvent * );

Atom get_atom_properties( const Client_t *client, const Atom property ) {

        // TODO: get_atom_properties() should return the number of items and a pointer to the stored data instead of this workaround
        Atom req_type = XA_ATOM;
        if ( property == g_xembed_atoms[ XEMBED_INFO ] ) {
                req_type = g_xembed_atoms[ XEMBED_INFO ];
        }

        Atom atom = None;
        unsigned char *pointer = NULL;

        Atom dummy_atom;
        int dummy_int;
        unsigned long dummy_long;

        if ( XGetWindowProperty( g_display, client->window, property, 0L, sizeof atom, False, req_type, &dummy_atom, &dummy_int, &dummy_long, &dummy_long, &pointer ) == Success && pointer ) {
                atom = *(Atom *) pointer;

                if ( dummy_atom == g_xembed_atoms[ XEMBED_INFO ] && dummy_long == 2 ) {
                        atom = ( (Atom *) pointer )[ 1 ];
                }

                XFree( pointer );
        }

        return atom;
}

// TODO: Clean up some of these variable names
Picture get_icon_properties( const Window window, unsigned int *icon_width, unsigned int *icon_height ) {
        int format;
        unsigned long n, extra, *p = NULL;
        Atom real;

        if ( XGetWindowProperty( g_display, window, g_net_atoms[ NET_WM_ICON ], 0L, LONG_MAX, False, AnyPropertyType, &real, &format, &n, &extra, (unsigned char **) &p ) != Success ) {
                return None;
        }

        if ( n == 0 || format != 32 ) {
                XFree( p );
                return None;
        }

        unsigned long *bstp = NULL;

        uint32_t w, h, sz;

        // @formatter:off
        { // @formatter:on

                unsigned long *i;
                const unsigned long *end = p + n;
                uint32_t bstd = UINT32_MAX, d, m;

                for ( i = p; i < end - 1; i += sz ) {
                        if ( ( w = *i++ ) >= 16384 || ( h = *i++ ) >= 16384 ) {
                                XFree( p );
                                return None;
                        }

                        if ( ( sz = w * h ) > end - i ) break;

                        if ( ( m = w > h ? w : h ) >= g_master_config.bar_config.client_icon_size && ( d = m - g_master_config.bar_config.client_icon_size ) < bstd ) {
                                bstd = d;
                                bstp = i;
                        }
                }

                if ( !bstp ) {
                        for ( i = p; i < end - 1; i += sz ) {
                                if ( ( w = *i++ ) >= 16384 || ( h = *i++ ) >= 16384 ) {
                                        XFree( p );
                                        return None;
                                }

                                if ( ( sz = w * h ) > end - i ) break;

                                if ( ( d = g_master_config.bar_config.client_icon_size - ( w > h ? w : h ) ) < bstd ) {
                                        bstd = d;
                                        bstp = i;
                                }
                        }
                }

                if ( !bstp ) {
                        XFree( p );
                        return None;
                }
        }

        if ( ( w = *( bstp - 2 ) ) == 0 || ( h = *( bstp - 1 ) ) == 0 ) {
                XFree( p );
                return None;
        }

        uint32_t new_icon_width, new_icon_height;
        if ( w <= h ) {
                new_icon_height = g_master_config.bar_config.client_icon_size;
                new_icon_width = w * g_master_config.bar_config.client_icon_size / h;

                if ( new_icon_width == 0 ) {
                        new_icon_width = 1;
                }
        } else {
                new_icon_width = g_master_config.bar_config.client_icon_size;
                new_icon_height = h * g_master_config.bar_config.client_icon_size / w;

                if ( new_icon_height == 0 ) {
                        new_icon_height = 1;
                }
        }

        *icon_width = new_icon_width;
        *icon_height = new_icon_height;

        uint32_t i, *bstp32 = (uint32_t *) bstp;
        for ( sz = w * h, i = 0; i < sz; ++i ) {
                bstp32[ i ] = pre_alpha( bstp[ i ] );
        }

        const Picture picture = drawable_picture_create_resized( g_drawable, (char *) bstp, w, h, new_icon_width, new_icon_height );

        XFree( p );
        return picture;
}

int get_root_pointer( int *x, int *y ) {
        int dummy_int;
        unsigned int dummy_uint;
        Window dummy_window;
        return XQueryPointer( g_display, g_root_window, &dummy_window, &dummy_window, x, y, &dummy_int, &dummy_int, &dummy_uint );
}

long get_state( const Window window ) {
        int dummy_int;
        unsigned long dummy_ulong;

        unsigned long n = 0;
        unsigned char *data = NULL;
        if ( XGetWindowProperty( g_display, window, g_window_manager_atoms[ WM_STATE ], 0L, 2L, False, g_window_manager_atoms[ WM_STATE ], &dummy_ulong, &dummy_int, &n, &dummy_ulong,
                                 &data ) != Success ) {
                return -1;
        }

        long result = -1;
        if ( n != 0 ) {
                result = *data;
        }

        XFree( data );
        return result;
}

int get_text_properties( const Window window, const Atom atom, char *text, const unsigned int size ) {
        if ( !text || size == 0 ) return 0;

        text[ 0 ] = '\0';

        XTextProperty text_property;
        if ( !XGetTextProperty( g_display, window, &text_property, atom ) || !text_property.nitems ) return 0;

        int text_count = 0;
        char **text_list = NULL;
        if ( text_property.encoding == XA_STRING ) {
                strncpy( text, (char *) text_property.value, size - 1 );
        } else if ( XmbTextPropertyToTextList( g_display, &text_property, &text_list, &text_count ) >= Success && text_count > 0 && *text_list ) {
                strncpy( text, *text_list, size - 1 );
                XFreeStringList( text_list );
        }

        text[ size - 1 ] = '\0';

        XFree( text_property.value );
        return 1;
}

void grab_buttons( const Client_t *client, const bool focused ) {
        update_num_lock_mask();
        XUngrabButton( g_display, AnyButton, AnyModifier, client->window );

        if ( !focused ) {
                XGrabButton( g_display, AnyButton, AnyModifier, client->window, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None );
        }

        const unsigned int modifiers[ ] = { 0, LockMask, g_num_lock_mask, g_num_lock_mask | LockMask };
        for ( unsigned int i = 0; i < g_master_config.buttonbinds_count; i++ ) {
                if ( g_master_config.buttonbinds[ i ].button != 0 && g_master_config.buttonbinds[ i ].click == CLICK_CLIENT_WINDOW ) {
                        for ( unsigned int j = 0; j < LENGTH( modifiers ); j++ ) {
                                XGrabButton( g_display, g_master_config.buttonbinds[ i ].button, g_master_config.buttonbinds[ i ].modifiers | modifiers[ j ], client->window, False, BUTTONMASK,
                                             GrabModeAsync, GrabModeSync, None, None );
                        }
                }
        }

}

void grab_keys( void ) {
        update_num_lock_mask();
        XUngrabKey( g_display, AnyKey, AnyModifier, g_root_window );

        const unsigned int modifiers[ ] = { 0, LockMask, g_num_lock_mask, g_num_lock_mask | LockMask };
        for ( unsigned int i = 0; i < g_master_config.keybinds_count; i++ ) {
                const KeyCode code = XKeysymToKeycode( g_display, g_master_config.keybinds[ i ].keysym );
                if ( g_master_config.keybinds[ i ].keysym != NoSymbol && code ) {
                        for ( unsigned int j = 0; j < LENGTH( modifiers ); j++ ) {
                                XGrabKey( g_display, code, g_master_config.keybinds[ i ].modifiers | modifiers[ j ], g_root_window, True, GrabModeAsync, GrabModeAsync );
                        }
                }
        }
}

uint32_t pre_alpha( const uint32_t p ) {
        uint8_t const a = p >> 24u;
        uint32_t const rb = ( a * ( p & 0xFF00FFu ) ) >> 8u;
        uint32_t const g = ( a * ( p & 0x00FF00u ) ) >> 8u;
        return ( rb & 0xFF00FFu ) | ( g & 0x00FF00u ) | ( a << 24u );
}

void update_num_lock_mask( void ) {
        g_num_lock_mask = 0;
        XModifierKeymap *modifier_keymap = XGetModifierMapping( g_display );

        for ( unsigned int i = 0; i < 8; i++ ) {
                for ( int j = 0; j < modifier_keymap->max_keypermod; j++ ) {
                        if ( modifier_keymap->modifiermap[ i * modifier_keymap->max_keypermod + j ] == XKeysymToKeycode( g_display, XK_Num_Lock ) ) {
                                g_num_lock_mask = ( 1 << i );
                        }
                }
        }

        XFreeModifiermap( modifier_keymap );
}

// TODO: Figure out some way to make this damn logic readable
int x_error( Display *display, XErrorEvent *error ) {

        // There's no way to check accesses to destroyed windows, thus those cases are
        // ignored (especially on UnmapNotify's). Other types of errors call Xlibs
        // default error handler, which may call exit.

        if ( error->error_code == BadWindow || ( error->request_code == X_SetInputFocus && error->error_code == BadMatch ) || ( error->request_code == X_PolyText8 && error->error_code == BadDrawable )
             || ( error->request_code == X_PolyFillRectangle && error->error_code == BadDrawable ) || ( error->request_code == X_PolySegment && error->error_code == BadDrawable ) || (
                     error->request_code == X_ConfigureWindow && error->error_code == BadMatch ) || ( error->request_code == X_GrabButton && error->error_code == BadAccess ) || (
                     error->request_code == X_GrabKey && error->error_code == BadAccess ) || ( error->request_code == X_CopyArea && error->error_code == BadDrawable ) ) return 0;

        log_error( "Request code=%d, error code=%d", error->request_code, error->error_code );

        return xerrorxlib( display, error ); /* may call exit */
}

int x_error_dummy( Display *display, XErrorEvent *error ) {
        return 0;
}

int x_error_start( Display *display, XErrorEvent *error ) {
        // Startup Error handler to check if another window manager is already running.
        log_fatal( "Another window manager is already running" );
}
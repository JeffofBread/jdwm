#include "systray.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "atoms.h"
#include "bar.h"
#include "client.h"
#include "core.h"
#include "draw.h"
#include "logger.h"
#include "monitor.h"
#include "parser.h"
#include "util.h"
#include "xevent.h"
#include "xproperty.h"
#include "xutil.h"

// TODO: Not sure about this custom struct. May be worth rolling into the bar config or something.
// If nothing else, move into the master config to reduce the number of global variables I think.
Systray_t *g_systray = NULL;

unsigned int get_systray_width( void ) {
        unsigned int width = 0;

        if ( g_master_config.bar_config.show_system_tray ) {
                for ( const Client_t *client = g_systray->icons; client; width += client->width + g_master_config.bar_config.system_tray_padding, client = client->next_client ) {}
        }

        return width ? width + g_master_config.bar_config.system_tray_padding : 1;
}

void remove_systray_icon( Client_t *icon ) {

        // https://www.reddit.com/r/suckless/comments/vdpq14/comment/icpsnhe/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
        XReparentWindow( g_display, icon->window, g_root_window, 0, 0 );

        if ( !g_master_config.bar_config.show_system_tray ) return;
        if ( !icon ) return;

        Client_t **icon_list;
        for ( icon_list = &g_systray->icons; *icon_list && *icon_list != icon; icon_list = &( *icon_list )->next_client ) {}

        if ( icon_list ) {
                *icon_list = icon->next_client;
        }

        free( icon );
}

Monitor_t *systray_to_monitor( Monitor_t *monitor ) {
        if ( !g_master_config.bar_config.system_tray_pinning ) {
                if ( !monitor ) {
                        return g_selected_monitor;
                }

                return monitor == g_selected_monitor ? monitor : NULL;
        }

        Monitor_t *tmp_monitor;

        // Counts how many monitors there are
        unsigned int monitors_count;
        for ( monitors_count = 1, tmp_monitor = g_monitors; tmp_monitor && tmp_monitor->next_monitor; monitors_count++, tmp_monitor = tmp_monitor->next_monitor ) {}

        // Iterates until it finds the monitor to pin the system tray to
        unsigned int i;
        for ( i = 1, tmp_monitor = g_monitors; tmp_monitor && tmp_monitor->next_monitor && i < g_master_config.bar_config.system_tray_pinning; i++, tmp_monitor = tmp_monitor->next_monitor ) {}

        if ( g_master_config.enable_system_tray_pin_to_first_monitor_on_fail && monitors_count < g_master_config.bar_config.system_tray_pinning ) {
                return g_monitors;
        }

        return tmp_monitor;
}

void update_systray( void ) {
        if ( !g_master_config.bar_config.show_system_tray ) return;

        XSetWindowAttributes window_attributes;
        Monitor_t *systray_monitor = systray_to_monitor( NULL );

        unsigned int systray_width = 1;
        unsigned int systray_x = systray_monitor->monitor_x + systray_monitor->monitor_width;

        if ( !g_systray ) {
                g_systray = ecalloc( 1, sizeof( Systray_t ) );
                g_systray->window = XCreateSimpleWindow( g_display, g_root_window, (int) systray_x - g_horizontal_bar_padding, systray_monitor->bar_y + g_vertical_bar_padding, systray_width,
                                                         g_bar_height, 0, 0, THEME.color_systray.pixel );

                window_attributes.event_mask = ButtonPressMask | ExposureMask;
                window_attributes.override_redirect = True;
                window_attributes.background_pixel = THEME.color_systray.pixel;

                XSelectInput( g_display, g_systray->window, SubstructureNotifyMask );
                XChangeProperty( g_display, g_systray->window, g_net_atoms[ NET_SYSTEM_TRAY_ORIENTATION ], XA_CARDINAL, 32, PropModeReplace,
                                 (unsigned char *) &g_net_atoms[ NET_SYSTEM_TRAY_ORIENTATION_HORZ ], 1 );
                XChangeWindowAttributes( g_display, g_systray->window, CWEventMask | CWOverrideRedirect | CWBackPixel, &window_attributes );
                XMapRaised( g_display, g_systray->window );
                XSetSelectionOwner( g_display, g_net_atoms[ NET_SYSTEM_TRAY ], g_systray->window, CurrentTime );

                if ( XGetSelectionOwner( g_display, g_net_atoms[ NET_SYSTEM_TRAY ] ) == g_systray->window ) {
                        send_event( g_root_window, g_xembed_atoms[ MANAGER ], StructureNotifyMask, CurrentTime, (long) g_net_atoms[ NET_SYSTEM_TRAY ], (long) g_systray->window, 0, 0 );
                        XSync( g_display, False );
                } else {
                        log_error( "Unable to obtain system tray" );
                        free( g_systray );
                        g_systray = NULL;
                        return;
                }
        }

        Client_t *icons;
        for ( systray_width = 0, icons = g_systray->icons; icons; icons = icons->next_client ) {

                // Make sure the background color stays the same
                window_attributes.background_pixel = THEME.color_systray.pixel;

                XChangeWindowAttributes( g_display, icons->window, CWBackPixel, &window_attributes );
                XMapRaised( g_display, icons->window );

                systray_width += g_master_config.bar_config.system_tray_padding;
                icons->x = (int) systray_width;

                XMoveResizeWindow( g_display, icons->window, icons->x, 0, icons->width, icons->height );

                systray_width += icons->width;

                if ( icons->monitor != systray_monitor ) {
                        icons->monitor = systray_monitor;
                }
        }

        systray_width = systray_width ? systray_width + g_master_config.bar_config.system_tray_padding : 1;
        systray_x -= systray_width;

        XMoveResizeWindow( g_display, g_systray->window, (int) systray_x - g_horizontal_bar_padding, systray_monitor->bar_y + g_vertical_bar_padding, systray_width, g_bar_height );

        XWindowChanges window_changes = {
                .x = (int) systray_x - g_horizontal_bar_padding,
                .y = systray_monitor->bar_y + g_vertical_bar_padding,
                .width = (int) systray_width,
                .height = g_bar_height,
                .stack_mode = Above,
                .sibling = systray_monitor->bar_window,
        };

        XConfigureWindow( g_display, g_systray->window, CWX | CWY | CWWidth | CWHeight | CWSibling | CWStackMode, &window_changes );
        XMapWindow( g_display, g_systray->window );
        XMapSubwindows( g_display, g_systray->window );

        // Redraw background
        XSetForeground( g_display, g_drawable->graphics_context, THEME.color_systray.pixel );
        XFillRectangle( g_display, g_systray->window, g_drawable->graphics_context, 0, 0, systray_width, g_bar_height );
        XSync( g_display, False );
}

void update_systray_icon_geometry( Client_t *icon, const int width, const int height ) {
        if ( icon ) {
                icon->height = g_bar_height;

                if ( width == height ) {
                        icon->width = g_bar_height;
                } else if ( height == g_bar_height ) {
                        icon->width = width;
                } else {
                        icon->width = (int) ( (float) g_bar_height * ( (float) width / (float) height ) );
                }

                apply_size_hints( icon, &( icon->x ), &( icon->y ), &( icon->width ), &( icon->height ), False );

                // Force icons into the systray dimensions if they don't want to
                if ( icon->height > g_bar_height ) {
                        if ( icon->width == icon->height ) {
                                icon->width = g_bar_height;
                        } else {
                                icon->width = (int) ( (float) g_bar_height * ( (float) icon->width / (float) icon->height ) );
                        }

                        icon->height = g_bar_height;
                }
        }
}

void update_systray_icon_state( Client_t *icon, const XPropertyEvent *event ) {
        if ( !g_master_config.bar_config.show_system_tray ) return;

        const Atom flags = get_atom_properties( icon, g_xembed_atoms[ XEMBED_INFO ] );

        if ( !icon || event->atom != g_xembed_atoms[ XEMBED_INFO ] || !flags ) return;

        int code = 0;
        if ( flags & XEMBED_MAPPED && !icon->tags ) {
                icon->tags = 1;
                code = XEMBED_WINDOW_ACTIVATE;
                XMapRaised( g_display, icon->window );
                set_client_state( icon, NormalState );
        } else if ( !( flags & XEMBED_MAPPED ) && icon->tags ) {
                icon->tags = 0;
                code = XEMBED_WINDOW_DEACTIVATE;
                XUnmapWindow( g_display, icon->window );
                set_client_state( icon, WithdrawnState );
        } else {
                return;
        }

        send_event( icon->window, g_xembed_atoms[ XEMBED ], StructureNotifyMask, CurrentTime, code, 0, (long) g_systray->window, XEMBED_EMBEDDED_VERSION );
}

Client_t *window_to_systray_icon( const Window window ) {
        Client_t *icon = NULL;
        if ( !g_master_config.bar_config.show_system_tray ) return icon;
        if ( !window ) return icon;
        for ( icon = g_systray->icons; icon && icon->window != window; icon = icon->next_client ) {}
        return icon;
}

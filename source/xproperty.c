#include "xproperty.h"

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "atoms.h"
#include "client.h"
#include "core.h"
#include "monitor.h"
#include "parser.h"
#include "tags.h"
#include "xevent.h"

void set_client_state( const Client_t *client, const long state ) {
        const long data[ ] = { state, None };
        XChangeProperty( g_display, client->window, g_window_manager_atoms[ WM_STATE ], g_window_manager_atoms[ WM_STATE ], 32, PropModeReplace, (unsigned char *) data, 2 );
}

void set_client_tag_property( const Client_t *client ) {
        const long data[ ] = { (long) client->tags, (long) client->monitor->monitor_number };
        XChangeProperty( g_display, client->window, g_net_atoms[ NET_CLIENT_INFO ], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) data, 2 );
}

void set_current_desktop( void ) {
        const long data[ ] = { 0 };
        XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_CURRENT_DESKTOP ], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) data, 1 );
}

void set_desktop_names( void ) {
        XTextProperty text;
        Xutf8TextListToTextProperty( g_display, g_master_config.bar_config.tags, TAGS_COUNT, XUTF8StringStyle, &text );
        XSetTextProperty( g_display, g_root_window, &text, g_net_atoms[ NET_DESKTOP_NAMES ] );
        if ( text.value ) XFree( text.value );
}

void set_focus( Client_t *client ) {
        if ( !client->should_never_focus ) {
                XSetInputFocus( g_display, client->window, RevertToPointerRoot, CurrentTime );
                XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_ACTIVE_WINDOW ], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &( client->window ), 1 );
        }

        send_event( client->window, g_window_manager_atoms[ WM_TAKE_FOCUS ], NoEventMask, (long) g_window_manager_atoms[ WM_TAKE_FOCUS ], CurrentTime, 0, 0, 0 );
}

void set_number_of_desktops( void ) {
        const long data[ ] = { TAGS_COUNT };
        XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_NUMBER_OF_DESKTOPS ], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) data, 1 );
}

void set_viewport( void ) {
        const long data[ ] = { 0, 0 };
        XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_DESKTOP_VIEWPORT ], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) data, 2 );
}

void update_client_list( void ) {
        XDeleteProperty( g_display, g_root_window, g_net_atoms[ NET_CLIENT_LIST ] );
        for ( const Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                for ( Client_t *client = monitor->client_list; client; client = client->next_client ) {
                        XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_CLIENT_LIST ], XA_WINDOW, 32, PropModeAppend, (unsigned char *) &( client->window ), 1 );
                }
        }
}

void update_current_desktop( void ) {
        const long rawdata[ ] = { g_selected_monitor->tag_set[ g_selected_monitor->selected_tags ] };

        int i = 0;
        while ( *rawdata >> ( i + 1 ) ) {
                i++;
        }

        const long data[ ] = { i };
        XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_CURRENT_DESKTOP ], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) data, 1 );
}

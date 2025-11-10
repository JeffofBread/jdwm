// TODO: These functions may need some renames

#include "atoms.h"

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include "core.h"
#include "xproperty.h"

Window g_net_window_manager_check_window = -1;
Atom g_window_manager_atoms[ WM_ATOMS_COUNT ], g_net_atoms[ NET_ATOMS_COUNT ], g_xembed_atoms[ XEMBED_ATOMS_COUNT ];

static Atom utf8_string = 0;

void initialize_atoms( void ) {
        utf8_string = XInternAtom( g_display, "UTF8_STRING", False );

        g_window_manager_atoms[ WM_PROTOCOLS ] = XInternAtom( g_display, "WM_PROTOCOLS", False );
        g_window_manager_atoms[ WM_DELETE ] = XInternAtom( g_display, "WM_DELETE_WINDOW", False );
        g_window_manager_atoms[ WM_STATE ] = XInternAtom( g_display, "WM_STATE", False );
        g_window_manager_atoms[ WM_TAKE_FOCUS ] = XInternAtom( g_display, "WM_TAKE_FOCUS", False );

        g_net_atoms[ NET_ACTIVE_WINDOW ] = XInternAtom( g_display, "_NET_ACTIVE_WINDOW", False );
        g_net_atoms[ NET_SUPPORTED ] = XInternAtom( g_display, "_NET_SUPPORTED", False );
        g_net_atoms[ NET_SYSTEM_TRAY ] = XInternAtom( g_display, "_NET_SYSTEM_TRAY_S0", False );
        g_net_atoms[ NET_SYSTEM_TRAY_OP ] = XInternAtom( g_display, "_NET_SYSTEM_TRAY_OPCODE", False );
        g_net_atoms[ NET_SYSTEM_TRAY_ORIENTATION ] = XInternAtom( g_display, "_NET_SYSTEM_TRAY_ORIENTATION", False );
        g_net_atoms[ NET_SYSTEM_TRAY_ORIENTATION_HORZ ] = XInternAtom( g_display, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False );
        g_net_atoms[ NET_WM_NAME ] = XInternAtom( g_display, "_NET_WM_NAME", False );
        g_net_atoms[ NET_WM_ICON ] = XInternAtom( g_display, "_NET_WM_ICON", False );
        g_net_atoms[ NET_WM_STATE ] = XInternAtom( g_display, "_NET_WM_STATE", False );
        g_net_atoms[ NET_WM_CHECK ] = XInternAtom( g_display, "_NET_SUPPORTING_WM_CHECK", False );
        g_net_atoms[ NET_WM_STATE_FULLSCREEN ] = XInternAtom( g_display, "_NET_WM_STATE_FULLSCREEN", False );
        g_net_atoms[ NET_WM_STATE_STICKY ] = XInternAtom( g_display, "_NET_WM_STATE_STICKY", False );
        g_net_atoms[ NET_WM_WINDOW_TYPE ] = XInternAtom( g_display, "_NET_WM_WINDOW_TYPE", False );
        g_net_atoms[ NET_WM_WINDOW_TYPE_DIALOG ] = XInternAtom( g_display, "_NET_WM_WINDOW_TYPE_DIALOG", False );
        g_net_atoms[ NET_WM_WINDOW_TYPE_DESKTOP ] = XInternAtom( g_display, "_NET_WM_WINDOW_TYPE_DESKTOP", False );
        g_net_atoms[ NET_CLIENT_LIST ] = XInternAtom( g_display, "_NET_CLIENT_LIST", False );
        g_net_atoms[ NET_DESKTOP_VIEWPORT ] = XInternAtom( g_display, "_NET_DESKTOP_VIEWPORT", False );
        g_net_atoms[ NET_NUMBER_OF_DESKTOPS ] = XInternAtom( g_display, "_NET_NUMBER_OF_DESKTOPS", False );
        g_net_atoms[ NET_CURRENT_DESKTOP ] = XInternAtom( g_display, "_NET_CURRENT_DESKTOP", False );
        g_net_atoms[ NET_DESKTOP_NAMES ] = XInternAtom( g_display, "_NET_DESKTOP_NAMES", False );

        g_xembed_atoms[ MANAGER ] = XInternAtom( g_display, "MANAGER", False );
        g_xembed_atoms[ XEMBED ] = XInternAtom( g_display, "_XEMBED", False );
        g_xembed_atoms[ XEMBED_INFO ] = XInternAtom( g_display, "_XEMBED_INFO", False );

        g_net_atoms[ NET_CLIENT_INFO ] = XInternAtom( g_display, "_NET_CLIENT_INFO", False );
}

void initialize_ewmh( void ) {
        XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_SUPPORTED ], XA_ATOM, 32, PropModeReplace, (unsigned char *) g_net_atoms, NET_ATOMS_COUNT );

        set_number_of_desktops();
        set_current_desktop();
        set_desktop_names();
        set_viewport();
        XDeleteProperty( g_display, g_root_window, g_net_atoms[ NET_CLIENT_LIST ] );
        XDeleteProperty( g_display, g_root_window, g_net_atoms[ NET_CLIENT_INFO ] );
}

void initialize_window_manager_check( void ) {
        g_net_window_manager_check_window = XCreateSimpleWindow( g_display, g_root_window, 0, 0, 1, 1, 0, 0, 0 );

        XChangeProperty( g_display, g_net_window_manager_check_window, g_net_atoms[ NET_WM_CHECK ], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &g_net_window_manager_check_window, 1 );
        XChangeProperty( g_display, g_net_window_manager_check_window, g_net_atoms[ NET_WM_NAME ], utf8_string, 8, PropModeReplace, (unsigned char *) "jdwm", 8 );
        XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_WM_CHECK ], XA_WINDOW, 32, PropModeReplace, (unsigned char *) &g_net_window_manager_check_window, 1 );
}

#ifndef ATOMS_H_
#define ATOMS_H_

#include <X11/X.h>

// Disable Jetbrains formatting for the following structs, because of
// this bug: https://github.com/intellij-rust/intellij-rust/issues/3657
//
// @formatter:off

enum Ewmh_Atoms {
        NET_ACTIVE_WINDOW,
        NET_WM_CHECK,
        NET_CLIENT_LIST,
        NET_SUPPORTED,
        NET_CURRENT_DESKTOP,
        NET_NUMBER_OF_DESKTOPS,
        NET_DESKTOP_NAMES,
        NET_DESKTOP_VIEWPORT,
        NET_SYSTEM_TRAY,
        NET_SYSTEM_TRAY_OP,
        NET_SYSTEM_TRAY_ORIENTATION,
        NET_SYSTEM_TRAY_ORIENTATION_HORZ,
        NET_CLIENT_INFO,
        NET_WM_NAME,
        NET_WM_ICON,
        NET_WM_STATE,
        NET_WM_STATE_STICKY,
        NET_WM_STATE_FULLSCREEN,
        NET_WM_WINDOW_TYPE,
        NET_WM_WINDOW_TYPE_DESKTOP,
        NET_WM_WINDOW_TYPE_DIALOG,
        NET_ATOMS_COUNT
};

enum Xembed_Atoms {
        MANAGER,
        XEMBED,
        XEMBED_INFO,
        XEMBED_ATOMS_COUNT
};

enum Jdwm_Atoms {
        WM_PROTOCOLS,
        WM_DELETE,
        WM_STATE,
        WM_TAKE_FOCUS,
        WM_ATOMS_COUNT
};

// @formatter:on

extern void initialize_atoms( void );
extern void initialize_ewmh( void );
extern void initialize_window_manager_check( void );

extern Window g_net_window_manager_check_window;
extern Atom g_window_manager_atoms[ WM_ATOMS_COUNT ], g_net_atoms[ NET_ATOMS_COUNT ], g_xembed_atoms[ XEMBED_ATOMS_COUNT ];

#endif /* ATOMS_H_ */

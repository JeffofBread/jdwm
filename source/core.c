#include "core.h"

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <sys/wait.h>
#include "atoms.h"
#include "bar.h"
#include "cli.h"
#include "client.h"
#include "common.h"
#include "draw.h"
#include "layout.h"
#include "logger.h"
#include "monitor.h"
#include "parser.h"
#include "systray.h"
#include "tags.h"
#include "util.h"
#include "xevent.h"
#include "xproperty.h"
#include "xutil.h"

// TODO: Order these somehow
Display *g_display = NULL;
Cursor *g_cursor[ CURSOR_COUNT ];
Window g_root_window = -1;
unsigned int g_logging_level = DEFAULT_LOGGER_LEVEL;
int g_unmanaged = 0;
int g_screen = -1;
int g_screen_width = -1, g_screen_height = -1;
int g_left_right_text_padding = -1;
bool g_restart = false, g_running = true;
int g_theme_index = 0;
char *g_user_home = NULL;

void check_for_other_window_manager( void ) {
        xerrorxlib = XSetErrorHandler( x_error_start );
        // This causes an error if some other window manager is running
        XSelectInput( g_display, DefaultRootWindow( g_display ), SubstructureRedirectMask );
        XSync( g_display, False );
        XSetErrorHandler( x_error );
        XSync( g_display, False );
}

void cleanup( void ) {
        const Layout_t empty_layout = { "", NULL, "" };
        CURRENT_SELECTED_TAG_INDEX.layout = &empty_layout;

        view_tag_bitmask( &( Argument_t){ . ui = ~0 } );

        for ( const Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                while ( monitor->client_stack ) unmanage( monitor->client_stack, 0 );
        }

        XUngrabKey( g_display, AnyKey, AnyModifier, g_root_window );

        while ( g_monitors ) cleanup_monitor( g_monitors );

        // Copied from bakkeby's dusk
        while ( g_systray->icons ) {
                remove_systray_icon( g_systray->icons );
        }

        if ( g_master_config.bar_config.show_system_tray ) {
                XUnmapWindow( g_display, g_systray->window );
                XDestroyWindow( g_display, g_systray->window );
                free( g_systray );
        }

        for ( int i = 0; i < CURSOR_COUNT; i++ ) {
                drawable_cursor_free( g_drawable, g_cursor[ i ] );
        }

        XDestroyWindow( g_display, g_net_window_manager_check_window );
        drawable_free( g_drawable );
        XSync( g_display, False );
        XSetInputFocus( g_display, PointerRoot, RevertToPointerRoot, CurrentTime );
        XDeleteProperty( g_display, g_root_window, g_net_atoms[ NET_ACTIVE_WINDOW ] );

        config_cleanup( &g_master_config );
}

extern void increment_theme( const Argument_t *argument ) {
        if ( !argument->i ) return;

        g_theme_index = ( g_theme_index + argument->i ) % (int) g_master_config.themes_count;

        if ( g_theme_index < 0 ) {
                g_theme_index += (int) g_master_config.themes_count;
        }

        draw_bars();
        update_borders();

        // TODO: Fix theming
        // Icons backgrounds go not update correctly, and there is no good way to force the
        // icons to update their image. Will need something like alpha to begin fixing this.
        update_systray();
}

void manage( const Window window, const XWindowAttributes *window_attributes ) {
        XWindowChanges window_changes;
        Window transient_window = None;
        const Client_t *tmp_client = NULL;

        Client_t *client = ecalloc( 1, sizeof( Client_t ) );
        client->window = window;

        if ( get_atom_properties( client, g_net_atoms[ NET_WM_WINDOW_TYPE ] ) == g_net_atoms[ NET_WM_WINDOW_TYPE_DESKTOP ] ) {
                XMapWindow( g_display, client->window );
                XLowerWindow( g_display, client->window );
                free( client );
                return;
        }

        // Geometry
        client->x = client->old_x = window_attributes->x;
        client->y = client->old_y = window_attributes->y;
        client->width = client->old_width = window_attributes->width;
        client->height = client->old_height = window_attributes->height;
        client->old_border_width = window_attributes->border_width;

        update_icon( client );
        update_title( client );
        update_size_hints( client );
        update_window_manager_hints( client );

        if ( XGetTransientForHint( g_display, window, &transient_window ) && ( ( tmp_client = window_to_client( transient_window ) ) ) ) {
                client->monitor = tmp_client->monitor;
                client->tags = tmp_client->tags;
        } else {
                client->monitor = g_selected_monitor;
                apply_rules( client );
        }

        if ( g_unmanaged ) {
                XMapWindow( g_display, client->window );

                if ( g_unmanaged == 1 ) {
                        XRaiseWindow( g_display, client->window );
                } else if ( g_unmanaged == 2 ) {
                        XLowerWindow( g_display, client->window );
                }

                update_window_manager_hints( client );

                if ( !client->should_never_focus ) {
                        XSetInputFocus( g_display, client->window, RevertToPointerRoot, CurrentTime );
                }

                send_event( client->window, g_window_manager_atoms[ WM_TAKE_FOCUS ], StructureNotifyMask, (long) g_window_manager_atoms[ WM_TAKE_FOCUS ], CurrentTime, 0, 0, 0 );

                free( client );
                g_unmanaged = 0;
                return;
        }

        update_window_type( client );

        // @formatter:off
        {
                unsigned long dummy_ulong;
                int dummy_int;
                Atom dummy_atom;

                unsigned long *data, n;
                if ( XGetWindowProperty( g_display, client->window, g_net_atoms[ NET_CLIENT_INFO ], 0L, 2L, False, XA_CARDINAL, &dummy_atom, &dummy_int, &n, &dummy_ulong, (unsigned char **) &data ) == Success && n == 2 ) {
                        client->tags = *data;
                        for ( Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                                if ( monitor->monitor_number == (int) *( data + 1 ) ) {
                                        client->monitor = monitor;
                                        break;
                                }
                        }
                }

                if ( n > 0 ) {
                        XFree( data );
                }
        }
        //@formatter:on

        set_client_tag_property( client );

        if ( client->x + WIDTH( client ) > client->monitor->window_x + client->monitor->window_width ) {
                client->x = client->monitor->window_x + client->monitor->window_width - WIDTH( client );
        }

        if ( client->y + HEIGHT( client ) > client->monitor->window_y + client->monitor->window_height ) {
                client->y = client->monitor->window_y + client->monitor->window_height - HEIGHT( client );
        }

        g_selected_monitor->tag_set[ g_selected_monitor->selected_tags ] &= ~g_scratchpad_tag;
        if ( !strcmp( client->name, g_master_config.window_config.scratchpad_name ) ) {
                client->monitor->tag_set[ client->monitor->selected_tags ] |= client->tags = g_scratchpad_tag;
        }

        if ( !client->is_floating ) {
                client->is_floating = ( client->old_state = ( transient_window != None || client->is_fixed ) );
        }

        if ( client->is_floating ) {
                client->border_width = g_master_config.window_config.floating_border_width;
                XRaiseWindow( g_display, client->window );
                XSetWindowBorder( g_display, window, THEME.color_client_unselected[ FLOATING_BORDER ].pixel );
        } else {
                client->border_width = g_master_config.window_config.normal_border_width;
                XSetWindowBorder( g_display, window, THEME.color_client_unselected[ NORMAL_BORDER ].pixel );
        }

        window_changes.border_width = client->border_width;

        client->x = client->monitor->monitor_x + ( client->monitor->monitor_width - WIDTH( client ) ) / 2;
        client->y = client->monitor->monitor_y + ( client->monitor->monitor_height - HEIGHT( client ) ) / 2;

        XConfigureWindow( g_display, window, CWBorderWidth, &window_changes );
        configure( client ); // propagates border_width, if size doesn't change
        XSelectInput( g_display, window, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask );
        grab_buttons( client, false );
        attach( client );
        attach_stack( client );
        XChangeProperty( g_display, g_root_window, g_net_atoms[ NET_CLIENT_LIST ], XA_WINDOW, 32, PropModeAppend, (unsigned char *) &( client->window ), 1 );
        XMoveResizeWindow( g_display, client->window, client->x + 2 * g_screen_width, client->y, client->width, client->height ); // some windows require this
        set_client_state( client, NormalState );

        if ( client->monitor == g_selected_monitor ) {
                fullscreen_check( g_selected_monitor->selected_client );
                unfocus( g_selected_monitor->selected_client, 0 );
        }

        client->monitor->selected_client = client;

        arrange( client->monitor );
        XMapWindow( g_display, client->window );

        if ( client && client->monitor == g_selected_monitor ) {
                XWarpPointer( g_display, None, client->window, 0, 0, 0, 0, client->width / 2, client->height / 2 );
        }

        focus( NULL );
}

void quit( const Argument_t *argument ) {
        g_restart = false;
        g_running = false;
}

void restart( const Argument_t *argument ) {
        g_restart = true;
        g_running = false;
}

void run( void ) {
        XSync( g_display, False );

        // Skip executing programs in `.jdwmrc` alongside jdwm
        if ( g_cli_flags[ FLAG_NO_JDWMRC ] ) {
                log_debug( "Skipping executing `.jdwmrc`" );
        } else {
                start_jdwmrc();
        }

        // Main event loop
        XEvent event;
        while ( g_running && !XNextEvent( g_display, &event ) ) {
                if ( handler[ event.type ] ) {
                        handler[ event.type ]( &event );
                }
        }
}

void scan( void ) {
        Window parent_window, child_window, *windows = NULL;
        XWindowAttributes wa;
        unsigned int count;

        if ( XQueryTree( g_display, g_root_window, &parent_window, &child_window, &windows, &count ) ) {
                int i;
                for ( i = 0; i < count; i++ ) {
                        if ( !XGetWindowAttributes( g_display, windows[ i ], &wa ) || wa.override_redirect || XGetTransientForHint( g_display, windows[ i ], &parent_window ) ) continue;
                        if ( wa.map_state == IsViewable || get_state( windows[ i ] ) == IconicState ) {
                                manage( windows[ i ], &wa );
                        }
                }

                // Now the transients
                for ( i = 0; i < count; i++ ) {
                        if ( !XGetWindowAttributes( g_display, windows[ i ], &wa ) ) continue;
                        if ( XGetTransientForHint( g_display, windows[ i ], &parent_window ) && ( wa.map_state == IsViewable || get_state( windows[ i ] ) == IconicState ) ) {
                                manage( windows[ i ], &wa );
                        }
                }

                if ( windows ) {
                        XFree( windows );
                }
        }
}

void setup( void ) {

        // Handle restart signals
        signal( SIGHUP, sighup );
        signal( SIGTERM, sigterm );

        // Do not transform children into zombies when they terminate
        struct sigaction signal_action;
        sigemptyset( &signal_action.sa_mask );
        signal_action.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
        signal_action.sa_handler = SIG_IGN;
        sigaction( SIGCHLD, &signal_action, NULL );

        // Clean up any zombies (inherited from .xinitrc etc.) immediately
        while ( waitpid( -1, NULL, WNOHANG ) > 0 ) {};

        // Java windows can sometimes not play will with WM, this helps alleviate that
        setenv( "_JAVA_AWT_WM_NONREPARENTING", "1", 1 );

        // Initialize screen
        if ( !drawable_font_create( g_drawable, THEME.font ) ) {
                log_fatal( "No fonts could be loaded" );
        }

        // TODO: Handle this logic better. Things like padding and height should be checked to make sure they dont get silly.
        // Example: g_bar_height could go negative with a negative vertical status padding and lead to negative bar height.
        g_left_right_text_padding = (int) g_drawable->fonts->h + g_master_config.bar_config.horizontal_bar_text_padding;
        g_bar_height = (int) g_drawable->fonts->h + g_master_config.bar_config.vertical_bar_text_padding;
        g_horizontal_bar_padding = g_master_config.bar_config.horizontal_bar_padding;
        g_vertical_bar_padding = ( g_master_config.bar_config.enable_top_bar == true ) ? g_master_config.bar_config.vertical_bar_padding : -g_master_config.bar_config.vertical_bar_padding;

        update_geometry();

        initialize_atoms();

        // Initialize cursors
        g_cursor[ CURSOR_NORMAL ] = drawable_cursor_create( g_drawable, XC_left_ptr );
        g_cursor[ CURSOR_RESIZE ] = drawable_cursor_create( g_drawable, XC_sizing );
        g_cursor[ CURSOR_MOVE ] = drawable_cursor_create( g_drawable, XC_fleur );

        // Initialize system tray
        update_systray();

        // Initialize bars
        update_bars();
        update_status();

        // Supporting window for NetWMCheck
        initialize_window_manager_check();

        // EWMH support per view
        initialize_ewmh();

        // Select events
        XSetWindowAttributes window_attributes = {
                .cursor = *g_cursor[ CURSOR_NORMAL ],
                .event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask,
        };

        XChangeWindowAttributes( g_display, g_root_window, CWEventMask | CWCursor, &window_attributes );
        XSelectInput( g_display, g_root_window, window_attributes.event_mask );
        grab_keys();
        focus( NULL );
}

void sigchld( int unused ) {
        if ( signal( SIGCHLD, sigchld ) == SIG_ERR ) {
                log_fatal( "Can't install SIGCHLD handler" );
        }
}

void sighup( int unused ) {
        quit( &( Argument_t){ . i = 1 } );
}

void sigterm( int unused ) {
        quit( &( Argument_t){ . i = 0 } );
}

void spawn( const Argument_t *argument ) {
        if ( fork() == 0 ) {
                if ( g_display ) {
                        close( ConnectionNumber( g_display ) );
                }

                setsid();

                struct sigaction signal_action;
                sigemptyset( &signal_action.sa_mask );
                signal_action.sa_flags = 0;
                signal_action.sa_handler = SIG_DFL;
                sigaction( SIGCHLD, &signal_action, NULL );

                char *argv[ ] = { "/bin/sh", "-c", (char *) argument->v, NULL };

                log_debug( "Attempting to spawn \"%s\"", (char *) argument->v );
                execvp( argv[ 0 ], argv );
                log_fatal( "execvp '%s' failed: %s", ( (char **) argv )[ 0 ], strerror( errno ) );
        }
}

void start_jdwmrc( void ) {

        char *config_home = get_xdg_config_home();
        if ( config_home == NULL ) {
                log_warn( "Unable to get home directory. Unable to start `.jdwmrc`" );
                return;
        }

        char jdwmrc_paths[ 3 ][ PATH_MAX ];
        snprintf( jdwmrc_paths[ 0 ], PATH_MAX, "%s/.jdwmrc", g_user_home );
        snprintf( jdwmrc_paths[ 1 ], PATH_MAX, "%s/.jdwmrc", config_home );
        snprintf( jdwmrc_paths[ 2 ], PATH_MAX, "%s/jdwm/.jdwmrc", config_home );

        free( config_home );

        bool jdwmrc_found = false;
        for ( int i = 0; i < LENGTH( jdwmrc_paths ); i++ ) {
                if ( access( jdwmrc_paths[ i ], X_OK ) == 0 ) {
                        jdwmrc_found = true;
                        log_debug( "Found and executing \".jdwmrc\" at \"%s\"", jdwmrc_paths[ i ] );
                        spawn( &( Argument_t){ . v = jdwmrc_paths[ i ] } );
                } else if ( access( jdwmrc_paths[ i ], F_OK ) == 0 ) {
                        jdwmrc_found = true;
                        log_warn( "Found \"%s\" but it is not executable. Use: chmod +x %s", jdwmrc_paths[ i ], jdwmrc_paths[ i ] );
                }
        }

        if ( !jdwmrc_found ) {
                log_warn( "No valid .jdwmrc file found in any configured path" );
        }
}

void update_borders( void ) {
        const Client_t *client = NULL;
        for ( const Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                for ( client = monitor->client_stack; client; client = client->next_client ) {

                        unsigned long border_color;
                        if ( client == monitor->selected_client ) {
                                border_color = THEME.color_client_selected[ client->is_floating ? FLOATING_BORDER : NORMAL_BORDER ].pixel;
                        } else {
                                border_color = THEME.color_client_unselected[ client->is_floating ? FLOATING_BORDER : NORMAL_BORDER ].pixel;
                        }

                        XSetWindowBorder( g_display, client->window, border_color );
                }
        }
}

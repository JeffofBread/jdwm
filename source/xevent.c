#include "xevent.h"

#include <stdbool.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "atoms.h"
#include "bar.h"
#include "client.h"
#include "common.h"
#include "core.h"
#include "draw.h"
#include "layout.h"
#include "monitor.h"
#include "parser.h"
#include "systray.h"
#include "tags.h"
#include "util.h"
#include "xproperty.h"
#include "xutil.h"

#define CLEANMASK( mask ) ( mask & ~( g_num_lock_mask | LockMask ) & ( ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask ) )

void ( *handler[ LASTEvent ] )( XEvent * ) = {
        [ButtonPress] = button_press,
        [ClientMessage] = client_message,
        [ConfigureRequest] = configure_request,
        [ConfigureNotify] = configure_notify,
        [DestroyNotify] = destroy_notify,
        [EnterNotify] = enter_notify,
        [Expose] = expose,
        [FocusIn] = focusin,
        [KeyPress] = keypress,
        [MappingNotify] = mapping_notify,
        [MapRequest] = map_request,
        [MotionNotify] = motion_notify,
        [PropertyNotify] = property_notify,
        [ResizeRequest] = resize_request,
        [UnmapNotify] = unmap_notify
};

void button_press( XEvent *event ) {
        TIMER_CREATE( buttonpress_timer );
        TIMER_INITIALIZE( buttonpress_timer );

        const XButtonPressedEvent *button_pressed_event = &event->xbutton;
        Monitor_t *monitor = window_to_monitor( button_pressed_event->window );

        // Focus monitor if necessary
        if ( monitor && monitor != g_selected_monitor ) {
                unfocus( g_selected_monitor->selected_client, 1 );
                g_selected_monitor = monitor;
                focus( NULL );
        }

        Argument_t argument = { 0 };
        Client_t *client = window_to_client( button_pressed_event->window );
        unsigned int click = CLICK_DESKTOP;
        if ( button_pressed_event->window == g_selected_monitor->bar_window ) {
                unsigned int x = 0, i = 0, occupied_tags = 0;

                for ( client = monitor->client_list; client; client = client->next_client ) occupied_tags |= client->tags;

                // Do not reserve space for vacant tags
                do {
                        if ( !( occupied_tags & 1 << i || monitor->tag_set[ monitor->selected_tags ] & 1 << i ) ) continue;

                        if ( g_master_config.bar_config.show_tags ) {
                                x += TEXT_WIDTH( g_master_config.bar_config.tags[ i ] );
                        }
                } while ( button_pressed_event->x >= x && ++i < TAGS_COUNT );

                if ( i < TAGS_COUNT && g_master_config.bar_config.show_tags ) {
                        click = CLICK_TAG;
                        argument.ui = 1 << i;
                } else if ( button_pressed_event->x < x + TEXT_WIDTH( CURRENT_SELECTED_TAG_INDEX.layout->symbol ) && g_master_config.bar_config.show_layout_symbol ) {
                        click = CLICK_LAYOUT_SYMBOL;
                } else if ( button_pressed_event->x > g_selected_monitor->window_width - (int) TEXT_WIDTH( g_status_text ) - get_systray_width() && g_master_config.bar_config.show_status_text ) {
                        click = CLICK_STATUS_TEXT;
                } else if ( g_master_config.bar_config.show_client_title ) {
                        click = CLICK_WINDOW_TITLE;
                }
        } else if ( client ) {
                focus( client );
                restack( g_selected_monitor );
                XAllowEvents( g_display, ReplayPointer, CurrentTime );
                click = CLICK_CLIENT_WINDOW;
        }

        for ( int i = 0; i < g_master_config.buttonbinds_count; i++ ) {
                if ( click == g_master_config.buttonbinds[ i ].click && g_master_config.buttonbinds[ i ].function && g_master_config.buttonbinds[ i ].button == button_pressed_event->button &&
                     CLEANMASK( g_master_config.buttonbinds[i].modifiers ) == CLEANMASK( button_pressed_event->state ) ) {
                        g_master_config.buttonbinds[ i ].function( click == CLICK_TAG && g_master_config.buttonbinds[ i ].argument.i == 0 ? &argument : &g_master_config.buttonbinds[ i ].argument );
                }
        }

        TIMER_CHECKPOINT( buttonpress_timer, "Buttonpress" );
}

void client_message( XEvent *event ) {
        const XClientMessageEvent *client_message_event = &event->xclient;

        Client_t *client = window_to_client( client_message_event->window );

        if ( g_master_config.bar_config.show_system_tray ) {
                if ( client_message_event->window == g_systray->window && client_message_event->message_type == g_net_atoms[ NET_SYSTEM_TRAY_OP ] ) {

                        // Add systray icons
                        const int SYSTEM_TRAY_REQUEST_DOCK = 0;
                        if ( client_message_event->data.l[ 1 ] == SYSTEM_TRAY_REQUEST_DOCK ) {
                                client = (Client_t *) ecalloc( 1, sizeof( Client_t ) );

                                if ( !( ( client->window = client_message_event->data.l[ 2 ] ) ) ) {
                                        SAFE_FREE( client );
                                        return;
                                }

                                client->monitor = g_selected_monitor;
                                client->next_client = g_systray->icons;
                                g_systray->icons = client;

                                XWindowAttributes window_attributes;
                                if ( !XGetWindowAttributes( g_display, client->window, &window_attributes ) ) {
                                        // Use sane defaults
                                        window_attributes.width = g_bar_height;
                                        window_attributes.height = g_bar_height;
                                        window_attributes.border_width = 0;
                                }

                                client->x = client->old_x = client->y = client->old_y = 0;
                                client->width = client->old_width = window_attributes.width;
                                client->height = client->old_height = window_attributes.height;
                                client->old_border_width = window_attributes.border_width;
                                client->border_width = 0;
                                client->is_floating = true;

                                // Reuse tags field as mapped status
                                client->tags = 1;

                                update_size_hints( client );
                                update_systray_icon_geometry( client, window_attributes.width, window_attributes.height );
                                XAddToSaveSet( g_display, client->window );
                                XSelectInput( g_display, client->window, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask );

                                // https://www.reddit.com/r/suckless/comments/hzop00/comment/fzpr3dc/?utm_source=share&utm_medium=web2x&context=3
                                XClassHint ch = { "jdwm_system_tray", "jdwm_system_tray" };
                                XSetClassHint( g_display, client->window, &ch );

                                XReparentWindow( g_display, client->window, g_systray->window, 0, 0 );

                                // Use parents background color
                                XSetWindowAttributes set_window_attributes;
                                set_window_attributes.background_pixel = THEME.color_systray.pixel;
                                XChangeWindowAttributes( g_display, client->window, CWBackPixel, &set_window_attributes );
                                send_event( client->window, g_net_atoms[ XEMBED ], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, (long) g_systray->window, XEMBED_EMBEDDED_VERSION );

                                // TODO: Not sure if I have to send these events, too
                                send_event( client->window, g_net_atoms[ XEMBED ], StructureNotifyMask, CurrentTime, XEMBED_FOCUS_IN, 0, (long) g_systray->window, XEMBED_EMBEDDED_VERSION );
                                send_event( client->window, g_net_atoms[ XEMBED ], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, (long) g_systray->window, XEMBED_EMBEDDED_VERSION );
                                send_event( client->window, g_net_atoms[ XEMBED ], StructureNotifyMask, CurrentTime, XEMBED_MODALITY_ON, 0, (long) g_systray->window, XEMBED_EMBEDDED_VERSION );

                                XSync( g_display, False );
                                resize_bar_window( g_selected_monitor );
                                update_systray();
                                set_client_state( client, NormalState );
                        }

                        return;
                }
        }

        if ( !client ) return;

        if ( client_message_event->message_type == g_net_atoms[ NET_WM_STATE ] ) {
                if ( client_message_event->data.l[ 1 ] == g_net_atoms[ NET_WM_STATE_FULLSCREEN ] || client_message_event->data.l[ 2 ] == g_net_atoms[ NET_WM_STATE_FULLSCREEN ] ) {
                        if ( client->fake_fullscreen_state == FAKE_FULLSCREEN_AND_NORMAL_FULLSCREEN && client->is_fullscreen ) {
                                client->fake_fullscreen_state = RESTORE_FAKE_FULLSCREEN;
                        }

                        set_fullscreen( client, ( client_message_event->data.l[ 0 ] == 1 /* _NET_WM_STATE_ADD    */
                                                  || ( client_message_event->data.l[ 0 ] == 2 /* _NET_WM_STATE_TOGGLE */ && !client->is_fullscreen ) ) );

                        if ( client_message_event->data.l[ 1 ] == g_net_atoms[ NET_WM_STATE_STICKY ] || client_message_event->data.l[ 2 ] == g_net_atoms[ NET_WM_STATE_STICKY ] ) {
                                set_sticky( client, ( client_message_event->data.l[ 0 ] == 1 || ( client_message_event->data.l[ 0 ] == 2 && !client->is_sticky ) ) );
                        }
                }
        } else if ( client_message_event->message_type == g_net_atoms[ NET_ACTIVE_WINDOW ] ) {
                if ( client != g_selected_monitor->selected_client && !client->is_urgent ) {
                        set_urgent( client, 1 );
                }
        }
}

void configure_notify( XEvent *event ) {
        const XConfigureEvent *configure_event = &event->xconfigure;

        if ( configure_event->window == g_root_window ) {
                const int dirty = ( g_screen_width != configure_event->width || g_screen_height != configure_event->height );

                g_screen_width = configure_event->width;
                g_screen_height = configure_event->height;

                // TODO: update_geometry() handling sucks, needs to be simplified
                if ( update_geometry() || dirty ) {
                        drawable_resize( g_drawable, g_screen_width, g_bar_height );
                        update_bars();

                        for ( Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                                for ( Client_t *client = monitor->client_list; client; client = client->next_client ) {
                                        if ( client->is_fullscreen && client->fake_fullscreen_state != 1 ) {
                                                resize_client( client, monitor->monitor_x, monitor->monitor_y, monitor->monitor_width, monitor->monitor_height );
                                        }
                                }

                                resize_bar_window( monitor );
                        }

                        focus( NULL );
                        arrange( NULL );
                }
        }
}

void configure_request( XEvent *event ) {
        const XConfigureRequestEvent *configure_request_event = &event->xconfigurerequest;

        Client_t *client = window_to_client( configure_request_event->window );
        if ( client ) {

                if ( configure_request_event->value_mask & CWBorderWidth ) {
                        client->border_width = configure_request_event->border_width;
                } else if ( client->is_floating || !CURRENT_SELECTED_TAG_INDEX.layout->arrange ) {

                        const Monitor_t *monitor = client->monitor;

                        if ( !client->is_steam ) {

                                if ( configure_request_event->value_mask & CWX ) {
                                        client->old_x = client->x;
                                        client->x = monitor->monitor_x + configure_request_event->x;
                                }

                                if ( configure_request_event->value_mask & CWY ) {
                                        client->old_y = client->y;
                                        client->y = monitor->monitor_y + configure_request_event->y;
                                }
                        }

                        if ( configure_request_event->value_mask & CWWidth ) {
                                client->old_width = client->width;
                                client->width = configure_request_event->width;
                        }

                        if ( configure_request_event->value_mask & CWHeight ) {
                                client->old_height = client->height;
                                client->height = configure_request_event->height;
                        }

                        // Center in x direction
                        if ( ( client->x + client->width ) > monitor->monitor_x + monitor->monitor_width && client->is_floating ) {
                                client->x = monitor->monitor_x + ( monitor->monitor_width / 2 - WIDTH( client ) / 2 );
                        }

                        // Center in y direction
                        if ( ( client->y + client->height ) > monitor->monitor_y + monitor->monitor_height && client->is_floating ) {
                                client->y = monitor->monitor_y + ( monitor->monitor_height / 2 - HEIGHT( client ) / 2 );
                        }

                        if ( ( configure_request_event->value_mask & ( CWX | CWY ) ) && !( configure_request_event->value_mask & ( CWWidth | CWHeight ) ) ) {
                                configure( client );
                        }

                        if ( ISVISIBLE( client ) ) {
                                XMoveResizeWindow( g_display, client->window, client->x, client->y, client->width, client->height );
                        }
                } else {
                        configure( client );
                }
        } else {
                XWindowChanges window_changes = {
                        .x = configure_request_event->x,
                        .y = configure_request_event->y,
                        .width = configure_request_event->width,
                        .height = configure_request_event->height,
                        .border_width = configure_request_event->border_width,
                        .sibling = configure_request_event->above,
                        .stack_mode = configure_request_event->detail,
                };

                XConfigureWindow( g_display, configure_request_event->window, configure_request_event->value_mask, &window_changes );
        }

        XSync( g_display, False );
}

void destroy_notify( XEvent *event ) {
        const XDestroyWindowEvent *destroy_window_event = &event->xdestroywindow;

        Client_t *client = window_to_systray_icon( destroy_window_event->window );
        if ( client ) {
                remove_systray_icon( client );
                resize_bar_window( g_selected_monitor );
                update_systray();
        }

        client = window_to_client( destroy_window_event->window );
        if ( client ) {
                unmanage( client, 1 );
        }
}

void enter_notify( XEvent *event ) {
        const XCrossingEvent *crossing_event = &event->xcrossing;

        if ( ( crossing_event->mode != NotifyNormal || crossing_event->detail == NotifyInferior ) && crossing_event->window != g_root_window ) return;

        Client_t *client = window_to_client( crossing_event->window );
        Monitor_t *monitor = client ? client->monitor : window_to_monitor( crossing_event->window );

        if ( monitor != g_selected_monitor ) {
                unfocus( g_selected_monitor->selected_client, 1 );
                g_selected_monitor = monitor;
        } else if ( !client || client == g_selected_monitor->selected_client ) {
                return;
        }

        focus( client );
}

void expose( XEvent *event ) {
        const XExposeEvent *expose_event = &event->xexpose;

        Monitor_t *monitor = window_to_monitor( expose_event->window );
        if ( expose_event->count == 0 && monitor ) {
                draw_bar( monitor );
                if ( monitor == g_selected_monitor ) {
                        update_systray();
                }
        }
}

void focusin( XEvent *event ) {
        // There are some broken focus acquiring clients needing extra handling
        const XFocusChangeEvent *focus_change_event = &event->xfocus;
        if ( g_selected_monitor->selected_client && focus_change_event->window != g_selected_monitor->selected_client->window ) {
                set_focus( g_selected_monitor->selected_client );
        }
}

void keypress( XEvent *event ) {
        TIMER_CREATE( keypress_timer );
        TIMER_INITIALIZE( keypress_timer );

        const XKeyEvent *key_event = &event->xkey;
        const KeySym keysym = XkbKeycodeToKeysym( g_display, key_event->keycode, 0, 0 );

        for ( unsigned int i = 0; i < g_master_config.keybinds_count; i++ ) {
                if ( keysym == g_master_config.keybinds[ i ].keysym && CLEANMASK( g_master_config.keybinds[i].modifiers ) == CLEANMASK( key_event->state ) ) {
                        g_master_config.keybinds[ i ].function( &( g_master_config.keybinds[ i ].argument ) );
                }
        }

        TIMER_CHECKPOINT( keypress_timer, "Keypress" );
}

void mapping_notify( XEvent *event ) {
        XMappingEvent *mapping_event = &event->xmapping;
        XRefreshKeyboardMapping( mapping_event );
        if ( mapping_event->request == MappingKeyboard ) grab_keys();
}

void map_request( XEvent *event ) {
        const XMapRequestEvent *map_request_event = &event->xmaprequest;
        const Client_t *icon = window_to_systray_icon( map_request_event->window );

        if ( icon ) {
                send_event( icon->window, g_net_atoms[ XEMBED ], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, (long) g_systray->window, XEMBED_EMBEDDED_VERSION );
                resize_bar_window( g_selected_monitor );
                update_systray();
        }

        static XWindowAttributes window_attributes;
        if ( !XGetWindowAttributes( g_display, map_request_event->window, &window_attributes ) || window_attributes.override_redirect ) {
                return;
        }

        if ( !window_to_client( map_request_event->window ) ) {
                manage( map_request_event->window, &window_attributes );
        }
}

void motion_notify( XEvent *event ) {
        static Monitor_t *static_monitor = NULL;

        const XMotionEvent *motion_event = &event->xmotion;
        Monitor_t *tmp_monitor = rectangle_to_monitor( motion_event->x_root, motion_event->y_root, 1, 1 );

        if ( motion_event->window != g_root_window ) return;

        if ( tmp_monitor != static_monitor && static_monitor ) {
                unfocus( g_selected_monitor->selected_client, 1 );
                g_selected_monitor = tmp_monitor;
                focus( NULL );
        }

        static_monitor = tmp_monitor;
}

void property_notify( XEvent *event ) {
        const XPropertyEvent *property_event = &event->xproperty;
        Client_t *icon = window_to_systray_icon( property_event->window );

        if ( icon ) {
                if ( property_event->atom == XA_WM_NORMAL_HINTS ) {
                        update_size_hints( icon );
                        update_systray_icon_geometry( icon, icon->width, icon->height );
                } else {
                        update_systray_icon_state( icon, property_event );
                }

                resize_bar_window( g_selected_monitor );
                update_systray();
        }

        if ( ( property_event->window == g_root_window ) && ( property_event->atom == XA_WM_NAME ) ) {
                update_status();
        } else if ( property_event->state == PropertyDelete ) {
                return; // ignored
        } else if ( ( icon = window_to_client( property_event->window ) ) ) {
                Window transient_window;

                switch ( property_event->atom ) {
                        case XA_WM_TRANSIENT_FOR:
                                if ( !icon->should_ignore_transient && !icon->is_floating && ( XGetTransientForHint( g_display, icon->window, &transient_window ) ) && ( (
                                             icon->is_floating = ( window_to_client( transient_window ) ) != NULL ) ) ) {
                                        arrange( icon->monitor );
                                }
                                break;

                        case XA_WM_NORMAL_HINTS:
                                icon->should_refresh_size_hints = true;
                                break;

                        case XA_WM_HINTS:
                                update_window_manager_hints( icon );
                                draw_bars();
                                break;

                        default:
                                break;
                }

                if ( property_event->atom == XA_WM_NAME || property_event->atom == g_net_atoms[ NET_WM_NAME ] ) {
                        update_title( icon );
                        if ( icon == icon->monitor->selected_client && g_master_config.bar_config.show_client_title ) {
                                draw_bar( icon->monitor );
                        }
                } else if ( property_event->atom == g_net_atoms[ NET_WM_ICON ] ) {
                        update_icon( icon );
                        if ( icon == icon->monitor->selected_client ) {
                                draw_bar( icon->monitor );
                        }
                }
                if ( property_event->atom == g_net_atoms[ NET_WM_WINDOW_TYPE ] ) {
                        update_window_type( icon );
                }
        }
}

void resize_request( XEvent *event ) {
        const XResizeRequestEvent *resize_request_event = &event->xresizerequest;

        Client_t *icon = window_to_systray_icon( resize_request_event->window );
        if ( icon ) {
                update_systray_icon_geometry( icon, resize_request_event->width, resize_request_event->height );
                resize_bar_window( g_selected_monitor );
                update_systray();
        }
}

int send_event( const Window window, const Atom protocol, const int mask, const long data_0, const long data_1, const long data_2, const long data_3, const long data_4 ) {
        Atom message_type;
        bool exists = false;

        if ( protocol == g_window_manager_atoms[ WM_TAKE_FOCUS ] || protocol == g_window_manager_atoms[ WM_DELETE ] ) {

                message_type = g_window_manager_atoms[ WM_PROTOCOLS ];

                int n;
                Atom *protocols;
                if ( XGetWMProtocols( g_display, window, &protocols, &n ) ) {
                        while ( !exists && n-- ) {
                                exists = protocols[ n ] == protocol;
                        }

                        XFree( protocols );
                }
        } else {
                exists = true;
                message_type = protocol;
        }

        if ( exists ) {
                XEvent event;
                event.type = ClientMessage;
                event.xclient.window = window;
                event.xclient.message_type = message_type;
                event.xclient.format = 32;
                event.xclient.data.l[ 0 ] = data_0;
                event.xclient.data.l[ 1 ] = data_1;
                event.xclient.data.l[ 2 ] = data_2;
                event.xclient.data.l[ 3 ] = data_3;
                event.xclient.data.l[ 4 ] = data_4;
                XSendEvent( g_display, window, False, mask, &event );
        }

        return exists;
}

void unmap_notify( XEvent *event ) {
        const XUnmapEvent *unmap_event = &event->xunmap;

        Client_t *client = window_to_client( unmap_event->window );
        if ( client ) {
                if ( unmap_event->send_event ) {
                        set_client_state( client, WithdrawnState );
                } else {
                        unmanage( client, 0 );
                }
        }
}

#include "client.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include "atoms.h"
#include "bar.h"
#include "common.h"
#include "core.h"
#include "draw.h"
#include "layout.h"
#include "monitor.h"
#include "parser.h"
#include "tags.h"
#include "xevent.h"
#include "xproperty.h"
#include "xutil.h"

const char BROKEN[ ] = "broken";
Client_t *g_last_focused_client = NULL;

// TODO: Is the rule check for steam still necessary?
void apply_rules( Client_t *client ) {

        // Set default values
        client->is_floating = false;
        client->tags = 0;
        client->old_floating_width = client->monitor->monitor_width - client->monitor->monitor_width / 4;
        client->old_floating_height = client->monitor->monitor_height - client->monitor->monitor_height / 4;
        client->old_floating_x = client->monitor->monitor_x + ( client->monitor->monitor_width / 2 ) - ( client->old_floating_width / 2 );
        client->old_floating_y = client->monitor->monitor_y + ( client->monitor->monitor_height / 2 ) - ( client->old_floating_height / 2 );

        XClassHint class_hint = { NULL, NULL };
        XGetClassHint( g_display, client->window, &class_hint );

        const char *class = class_hint.res_class ? class_hint.res_class : BROKEN;
        const char *instance = class_hint.res_name ? class_hint.res_name : BROKEN;

        // Steam can often cause issues, so we specifically target and remember it and its apps
        if ( strstr( class, "Steam" ) || strstr( class, "steam_app_" ) ) client->is_steam = true;

        Monitor_t *monitor = NULL;
        for ( unsigned int i = 0; i < g_master_config.rules_count; i++ ) {
                const Rule_t *rule = &g_master_config.rules[ i ];

                // TODO: Make this statement a little more readable
                if ( ( rule->title[ 0 ] == '\0' || strstr( client->name, rule->title ) ) && ( rule->class[ 0 ] == '\0' || strstr( class, rule->class ) ) && (
                             rule->instance[ 0 ] == '\0' || strstr( instance, rule->instance ) ) ) {
                        client->should_ignore_transient = rule->ignore_transient;
                        client->is_floating = rule->is_floating;
                        client->tags |= rule->tags;
                        g_unmanaged = rule->unmanaged;
                        for ( monitor = g_monitors; monitor && monitor->monitor_number != rule->monitor; monitor = monitor->next_monitor ) {}
                        if ( monitor ) client->monitor = monitor;
                }

                // Un-manage steam toast notifications to try and stop them screwing with tiled windows
                if ( strstr( client->name, "notificationtoasts_" ) ) g_unmanaged = 1;
        }

        if ( class_hint.res_class ) XFree( class_hint.res_class );
        if ( class_hint.res_name ) XFree( class_hint.res_name );

        client->tags = client->tags & TAGMASK ? client->tags & TAGMASK : client->monitor->tag_set[ client->monitor->selected_tags ];
}

int apply_size_hints( Client_t *client, int *x, int *y, int *width, int *height, const int interact ) {
        const Monitor_t *monitor = client->monitor;

        // Set minimum possible values
        *width = MAX( 1, *width );
        *height = MAX( 1, *height );

        if ( interact ) {
                if ( *x > g_screen_width ) *x = g_screen_width - WIDTH( client );
                if ( *y > g_screen_height ) *y = g_screen_height - HEIGHT( client );
                if ( *x + *width + 2 * client->border_width < 0 ) *x = 0;
                if ( *y + *height + 2 * client->border_width < 0 ) *y = 0;
        } else {
                if ( *x >= monitor->window_x + monitor->window_width ) *x = monitor->window_x + monitor->window_width - WIDTH( client );
                if ( *y >= monitor->window_y + monitor->window_height ) *y = monitor->window_y + monitor->window_height - HEIGHT( client );
                if ( *x + *width + 2 * client->border_width <= monitor->window_x ) *x = monitor->window_x;
                if ( *y + *height + 2 * client->border_width <= monitor->window_y ) *y = monitor->window_y;
        }

        if ( *height < g_bar_height ) *height = g_bar_height;
        if ( *width < g_bar_height ) *width = g_bar_height;

        if ( !g_master_config.layout_config.respect_resize_hints && ( client->is_floating || !client->monitor->tags[ client->monitor->current_tag_index ].layout->arrange ) ) {

                if ( client->should_refresh_size_hints ) update_size_hints( client );

                // See last two sentences in ICCCM 4.1.2.3
                const int base_is_minimum_size = client->base_width == client->min_width && client->base_height == client->min_height;

                // Temporarily remove base dimensions
                if ( !base_is_minimum_size ) {
                        *width -= client->base_width;
                        *height -= client->base_height;
                }

                // Adjust for aspect limits
                if ( client->min_aspect_ratio > 0 && client->max_aspect_ratio > 0 ) {
                        if ( client->max_aspect_ratio < (double) *width / (double) *height ) *width = (int) ( *height * client->max_aspect_ratio + 0.5 );
                        else if ( client->min_aspect_ratio < (double) *height / (double) *width ) *height = (int) ( *width * client->min_aspect_ratio + 0.5 );
                }

                // Increment calculation requires this
                if ( base_is_minimum_size ) {
                        *width -= client->base_width;
                        *height -= client->base_height;
                }

                // Adjust for increment value
                if ( client->increment_width ) *width -= *width % client->increment_width;
                if ( client->increment_height ) *height -= *height % client->increment_height;

                // Restore base dimensions
                *width = MAX( *width + client->base_width, client->min_width );
                *height = MAX( *height + client->base_height, client->min_height );

                if ( client->max_width ) *width = MIN( *width, client->max_width );
                if ( client->max_height ) *height = MIN( *height, client->max_height );
        }

        return *x != client->x || *y != client->y || *width != client->width || *height != client->height;
}

void attach( Client_t *client ) {
        client->next_client = client->monitor->client_list;
        client->monitor->client_list = client;
}

void attach_stack( Client_t *client ) {
        client->stacking_next_client = client->monitor->client_stack;
        client->monitor->client_stack = client;
}

void client_and_view_to_tag( const Argument_t *argument ) {
        client_to_tag( argument );
        view_tag( argument );
}

void client_and_view_to_tag_bitmask( const Argument_t *argument ) {
        client_to_tag_bitmask( argument );
        view_tag_bitmask( argument );
}

void client_to_tag( const Argument_t *argument ) {
        const Argument_t tag_bitmask = { .ui = ( 1 << ( argument->ui - 1 ) ) };
        client_to_tag_bitmask( &tag_bitmask );
}

void client_to_tag_bitmask( const Argument_t *argument ) {
        if ( g_selected_monitor->selected_client && argument->ui & TAGMASK ) {
                Client_t *client = g_selected_monitor->selected_client;
                g_selected_monitor->selected_client->tags = argument->ui & TAGMASK;
                set_client_tag_property( client );
                focus( NULL );
                arrange( g_selected_monitor );
        }
}

void center_floating( const Argument_t *argument ) {
        Client_t *client = g_selected_monitor->selected_client;

        if ( client->is_floating ) {
                const int new_x = client->monitor->monitor_x + ( client->monitor->monitor_width - WIDTH( client ) ) / 2;
                const int new_y = client->monitor->monitor_y + ( client->monitor->monitor_height - HEIGHT( client ) ) / 2;

                resize( client, new_x, new_y, client->width, client->height, 0 );
                XWarpPointer( g_display, None, client->monitor->selected_client->window, 0, 0, 0, 0, client->monitor->selected_client->width / 2, client->monitor->selected_client->height / 2 );
        }
}

// TODO: Investigate why `is_fullscreen` is being messed with here
void client_to_monitor( const Argument_t *argument ) {
        Client_t *client = g_selected_monitor->selected_client;

        if ( !client || !g_monitors->next_monitor ) return;

        if ( client->is_fullscreen ) {
                client->is_fullscreen = false;
                send_to_monitor( client, direction_to_next_monitor( argument->i ) );
                client->is_fullscreen = true;

                if ( client->fake_fullscreen_state != 1 ) {
                        resize_client( client, client->monitor->monitor_x, client->monitor->monitor_y, client->monitor->monitor_width, client->monitor->monitor_height );
                        XRaiseWindow( g_display, client->window );
                }
        } else {
                send_to_monitor( client, direction_to_next_monitor( argument->i ) );
        }
}

void configure( const Client_t *client ) {

        XConfigureEvent configure_event = {
                .type = ConfigureNotify,
                .display = g_display,
                .event = client->window,
                .window = client->window,
                .x = client->x,
                .y = client->y,
                .width = client->width,
                .height = client->height,
                .border_width = client->border_width,
                .above = None,
                .override_redirect = False,
        };

        XSendEvent( g_display, client->window, False, StructureNotifyMask, (XEvent *) &configure_event );
}

void detach( const Client_t *client ) {
        Client_t **tmp_client_list;
        for ( tmp_client_list = &client->monitor->client_list; *tmp_client_list && *tmp_client_list != client; tmp_client_list = &( *tmp_client_list )->next_client ) {}
        *tmp_client_list = client->next_client;
}

void detach_stack( const Client_t *client ) {
        Client_t **tmp_client_stack;
        for ( tmp_client_stack = &client->monitor->client_stack; *tmp_client_stack && *tmp_client_stack != client; tmp_client_stack = &( *tmp_client_stack )->stacking_next_client ) {}
        *tmp_client_stack = client->stacking_next_client;

        if ( client == client->monitor->selected_client ) {
                Client_t *tmp_client;
                for ( tmp_client = client->monitor->client_stack; tmp_client && !ISVISIBLE( tmp_client ); tmp_client = tmp_client->stacking_next_client ) {}
                client->monitor->selected_client = tmp_client;
        }
}

void enqueue( Client_t *client ) {
        Client_t *tmp_client_list;
        for ( tmp_client_list = client->monitor->client_list; tmp_client_list && tmp_client_list->next_client; tmp_client_list = tmp_client_list->next_client ) {}
        if ( tmp_client_list ) {
                tmp_client_list->next_client = client;
                client->next_client = NULL;
        }
}

void enqueue_stack( Client_t *client ) {
        Client_t *tmp_client_list;
        for ( tmp_client_list = client->monitor->client_stack; tmp_client_list && tmp_client_list->stacking_next_client; tmp_client_list = tmp_client_list->stacking_next_client ) {}
        if ( tmp_client_list ) {
                tmp_client_list->stacking_next_client = client;
                client->stacking_next_client = NULL;
        }
}

void float_and_move( const Argument_t *argument ) {
        if ( g_selected_monitor->selected_client && !g_selected_monitor->selected_client->is_floating ) toggle_floating( NULL );
        move_mouse( argument );
}

void focus( Client_t *client ) {

        if ( !client || !ISVISIBLE( client ) ) {
                for ( client = g_selected_monitor->client_stack; client && !ISVISIBLE( client ); client = client->stacking_next_client ) {}
        }

        if ( g_selected_monitor->selected_client && g_selected_monitor->selected_client != client ) {
                lose_fullscreen( client );
                unfocus( g_selected_monitor->selected_client, 0 );
        }

        if ( client ) {
                if ( client->monitor != g_selected_monitor ) {
                        g_selected_monitor = client->monitor;
                }

                if ( client->is_urgent ) {
                        set_urgent( client, 0 );
                }

                detach_stack( client );
                attach_stack( client );
                grab_buttons( client, true );

                // Set new focused border first to avoid flickering
                if ( client->is_floating ) {
                        XSetWindowBorder( g_display, client->window, THEME.color_client_selected[ FLOATING_BORDER ].pixel );
                        if ( g_master_config.window_config.enable_raise_floating_windows_on_focus ) {
                                XRaiseWindow( g_display, client->window );
                        }
                } else {
                        XSetWindowBorder( g_display, client->window, THEME.color_client_selected[ NORMAL_BORDER ].pixel );
                }

                if ( g_last_focused_client && g_last_focused_client != client ) {
                        XSetWindowBorder( g_display, g_last_focused_client->window, THEME.color_client_unselected[ NORMAL_BORDER ].pixel );
                }

                set_focus( client );
        } else {
                XSetInputFocus( g_display, g_root_window, RevertToPointerRoot, CurrentTime );
                XDeleteProperty( g_display, g_root_window, g_net_atoms[ NET_ACTIVE_WINDOW ] );
        }

        g_selected_monitor->selected_client = client;
        draw_bars();
}

void focus_stack( const Argument_t *argument ) {
        Client_t *client = NULL;

        if ( !g_selected_monitor->selected_client || ( g_selected_monitor->selected_client->is_fullscreen && g_selected_monitor->selected_client->fake_fullscreen_state != 1 ) ) return;

        if ( argument->i > 0 ) {
                for ( client = g_selected_monitor->selected_client->next_client; client && !ISVISIBLE( client ); client = client->next_client ) {}

                if ( !client ) {
                        for ( client = g_selected_monitor->client_list; client && !ISVISIBLE( client ); client = client->next_client ) {}
                }
        } else {
                Client_t *tmp_client;

                for ( tmp_client = g_selected_monitor->client_list; tmp_client != g_selected_monitor->selected_client; tmp_client = tmp_client->next_client ) {
                        if ( ISVISIBLE( tmp_client ) ) {
                                client = tmp_client;
                        }
                }

                if ( !client ) {
                        for ( ; tmp_client; tmp_client = tmp_client->next_client ) {
                                if ( ISVISIBLE( tmp_client ) ) client = tmp_client;
                        }
                }
        }

        if ( client ) {
                focus( client );
                restack( g_selected_monitor );
                XWarpPointer( g_display, None, client->window, 0, 0, 0, 0, client->width / 2, client->height / 2 );
        }
}

// TODO: Maybe move this to draw.c?
void free_icon( Client_t *client ) {
        if ( client->icon ) {
                XRenderFreePicture( g_display, client->icon );
                client->icon = None;
        }
}

void fullscreen_check( Client_t *client ) {
        if ( !client ) return;

        if ( client->fake_fullscreen_state == NO_FAKE_FULLSCREEN && client->is_fullscreen ) {
                toggle_fake_fullscreen( 0 );
        } else if ( client->fake_fullscreen_state == FAKE_FULLSCREEN_AND_NORMAL_FULLSCREEN ) {
                toggle_fullscreen( 0 );
        }
}

void kill_client( const Argument_t *argument ) {
        if ( !g_selected_monitor->selected_client ) return;

        if ( !send_event( g_selected_monitor->selected_client->window, g_window_manager_atoms[ WM_DELETE ], NoEventMask, (long) g_window_manager_atoms[ WM_DELETE ], CurrentTime, 0, 0, 0 ) ) {
                XGrabServer( g_display );
                XSetErrorHandler( x_error_dummy );
                XSetCloseDownMode( g_display, DestroyAll );
                XKillClient( g_display, g_selected_monitor->selected_client->window );
                XSync( g_display, False );
                XSetErrorHandler( x_error );
                XUngrabServer( g_display );
        }
}

void lose_fullscreen( Client_t *next_client ) {
        if ( !g_selected_monitor->selected_client || !next_client ) return;

        if ( g_selected_monitor->selected_client->is_fullscreen && g_selected_monitor->selected_client->fake_fullscreen_state != 1 && ISVISIBLE( g_selected_monitor->selected_client ) &&
             g_selected_monitor->selected_client->monitor == next_client->monitor && !next_client->is_floating ) {
                set_fullscreen( g_selected_monitor->selected_client, false );
        }
}

void move_mouse( const Argument_t *argument ) {
        int x, y, new_x, new_y;
        Client_t *client;
        Monitor_t *monitor;
        XEvent event;
        Time last_time = 0;

        if ( !( ( client = g_selected_monitor->selected_client ) ) ) return;

        // No support moving fullscreen windows by mouse
        if ( client->is_fullscreen && client->fake_fullscreen_state != 1 ) return;

        // Don't move windows that aren't floating
        if ( client->is_floating != 1 ) return;

        restack( g_selected_monitor );

        const int old_x = client->x;
        const int old_y = client->y;

        if ( XGrabPointer( g_display, g_root_window, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, *g_cursor[ CURSOR_MOVE ], CurrentTime ) != GrabSuccess ) return;

        if ( !get_root_pointer( &x, &y ) ) return;

        do {
                XMaskEvent( g_display, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &event );
                switch ( event.type ) {
                        case ConfigureRequest:
                        case Expose:
                        case MapRequest:
                                handler[ event.type ]( &event );
                                break;
                        case MotionNotify:
                                if ( ( event.xmotion.time - last_time ) <= ( 1000 / g_master_config.mouse_refresh_rate ) ) continue;

                                last_time = event.xmotion.time;

                                new_x = old_x + ( event.xmotion.x - x );
                                new_y = old_y + ( event.xmotion.y - y );

                                if ( abs( g_selected_monitor->window_x - new_x ) < g_master_config.window_config.snapping_pixel_distance ) {
                                        new_x = g_selected_monitor->window_x;
                                } else if ( abs( ( g_selected_monitor->window_x + g_selected_monitor->window_width ) - ( new_x + WIDTH( client ) ) ) < g_master_config.window_config.
                                            snapping_pixel_distance ) {
                                        new_x = g_selected_monitor->window_x + g_selected_monitor->window_width - WIDTH( client );
                                }

                                if ( abs( g_selected_monitor->window_y - new_y ) < g_master_config.window_config.snapping_pixel_distance ) {
                                        new_y = g_selected_monitor->window_y;
                                } else if ( abs( ( g_selected_monitor->window_y + g_selected_monitor->window_height ) - ( new_y + HEIGHT( client ) ) ) < g_master_config.window_config.
                                            snapping_pixel_distance ) {
                                        new_y = g_selected_monitor->window_y + g_selected_monitor->window_height - HEIGHT( client );
                                }

                                if ( !client->is_floating && CURRENT_SELECTED_TAG_INDEX.layout->arrange && (
                                             abs( new_x - client->x ) > g_master_config.window_config.snapping_pixel_distance || abs( new_y - client->y ) > g_master_config.window_config.
                                             snapping_pixel_distance ) ) {
                                        toggle_floating( NULL );
                                }

                                if ( !CURRENT_SELECTED_TAG_INDEX.layout->arrange || client->is_floating ) {
                                        resize( client, new_x, new_y, client->width, client->height, 1 );
                                }

                                break;
                        default:
                                break;
                }
        } while ( event.type != ButtonRelease );

        XUngrabPointer( g_display, CurrentTime );

        if ( ( monitor = rectangle_to_monitor( client->x, client->y, client->width, client->height ) ) != g_selected_monitor ) {
                send_to_monitor( client, monitor );
                g_selected_monitor = monitor;
                focus( NULL );
        }
}

Client_t *next_tiled( Client_t *client ) {
        for ( ; client && ( client->is_floating || !ISVISIBLE( client ) ); client = client->next_client ) {}
        return client;
}

void pop( Client_t *client ) {
        detach( client );
        attach( client );
        focus( client );
        arrange( client->monitor );
}

void resize( Client_t *client, int x, int y, int w, int h, const int interact ) {
        if ( apply_size_hints( client, &x, &y, &w, &h, interact ) ) resize_client( client, x, y, w, h );
}

void resize_client( Client_t *client, const int x, const int y, const int w, const int h ) {
        XWindowChanges window_changes;

        client->old_x = client->x;
        client->old_y = client->y;
        client->old_width = client->width;
        client->old_height = client->height;

        client->x = window_changes.x = x;
        client->y = window_changes.y = y;
        client->width = window_changes.width = w;
        client->height = window_changes.height = h;

        window_changes.border_width = client->border_width;

        XConfigureWindow( g_display, client->window, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &window_changes );
        configure( client );
        XSync( g_display, False );
}

// TODO: Rename. Resize mouse is confusing
void resize_mouse( const Argument_t *argument ) {

        Client_t *client;
        Monitor_t *monitor;
        XEvent event;
        Time last_time = 0;
        int new_x, new_y, new_width, new_height;
        int horizontal_corner = 0, vertical_corner = 0;

        if ( !( ( client = g_selected_monitor->selected_client ) ) ) return;

        // No support resizing fullscreen windows by mouse
        if ( client->is_fullscreen && client->fake_fullscreen_state != 1 ) return;

        restack( g_selected_monitor );

        const int old_x = client->x;
        const int old_y = client->y;
        const int old_x_and_width = client->x + client->width;
        const int old_y_and_height = client->y + client->height;

        if ( XGrabPointer( g_display, g_root_window, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, *g_cursor[ CURSOR_RESIZE ], CurrentTime ) != GrabSuccess ) return;

        if ( client->is_floating || NULL == client->monitor->tags[ client->monitor->current_tag_index ].layout->arrange ) {

                Window dummy_window;
                int dummy_int;
                unsigned int dummy_uint;
                if ( !XQueryPointer( g_display, client->window, &dummy_window, &dummy_window, &dummy_int, &dummy_int, &new_x, &new_y, &dummy_uint ) ) return;

                horizontal_corner = new_x < client->width / 2;
                vertical_corner = new_y < client->height / 2;

                XWarpPointer( g_display, None, client->window, 0, 0, 0, 0, horizontal_corner ? ( -client->border_width ) : ( client->width + client->border_width - 1 ),
                              vertical_corner ? ( -client->border_width ) : ( client->height + client->border_width - 1 ) );
        } else {
                XWarpPointer( g_display, None, g_root_window, 0, 0, 0, 0,
                              g_selected_monitor->monitor_x + (int) ( (double) g_selected_monitor->window_width * CURRENT_SELECTED_TAG_INDEX.master_factor ),
                              g_selected_monitor->monitor_y + ( g_selected_monitor->window_height / 2 ) );
        }

        do {
                XMaskEvent( g_display, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &event );
                switch ( event.type ) {
                        case ConfigureRequest:
                        case Expose:
                        case MapRequest:
                                handler[ event.type ]( &event );
                                break;
                        case MotionNotify:
                                if ( ( event.xmotion.time - last_time ) <= ( 1000 / 60 ) ) continue;
                                last_time = event.xmotion.time;

                                new_x = horizontal_corner ? event.xmotion.x : client->x;
                                new_y = vertical_corner ? event.xmotion.y : client->y;
                                new_width = MAX( horizontal_corner ? ( old_x_and_width - new_x ) : ( event.xmotion.x - old_x - 2 * client->border_width + 1 ), 1 );
                                new_height = MAX( vertical_corner ? ( old_y_and_height - new_y ) : ( event.xmotion.y - old_y - 2 * client->border_width + 1 ), 1 );

                                if ( !CURRENT_SELECTED_TAG_INDEX.layout->arrange || client->is_floating ) {
                                        resize( client, new_x, new_y, new_width, new_height, 1 );
                                }

                                break;
                        default:
                                break;
                }
        } while ( event.type != ButtonRelease );

        if ( client->is_floating || NULL == client->monitor->tags[ client->monitor->current_tag_index ].layout->arrange ) {
                XWarpPointer( g_display, None, client->window, 0, 0, 0, 0, horizontal_corner ? ( -client->border_width ) : ( client->width + client->border_width - 1 ),
                              vertical_corner ? ( -client->border_width ) : ( client->height + client->border_width - 1 ) );
        } else {
                CURRENT_SELECTED_TAG_INDEX.master_factor = (double) ( event.xmotion.x_root - g_selected_monitor->monitor_x ) / (double) g_selected_monitor->window_width;
                arrange( g_selected_monitor );
                XWarpPointer( g_display, None, g_root_window, 0, 0, 0, 0,
                              g_selected_monitor->monitor_x + (int) ( (double) g_selected_monitor->window_width * CURRENT_SELECTED_TAG_INDEX.master_factor ),
                              g_selected_monitor->monitor_y + ( g_selected_monitor->window_height / 2 ) );
        }

        XUngrabPointer( g_display, CurrentTime );

        while ( XCheckMaskEvent( g_display, EnterWindowMask, &event ) ) {}

        if ( ( monitor = rectangle_to_monitor( client->x, client->y, client->width, client->height ) ) != g_selected_monitor ) {
                send_to_monitor( client, monitor );
                g_selected_monitor = monitor;
                focus( NULL );
        }
}

void rotate_stack( const Argument_t *argument ) {
        if ( !g_selected_monitor->selected_client ) return;

        Client_t *client = NULL, *focus_client = g_selected_monitor->selected_client;
        if ( argument->i > 0 ) {
                for ( client = next_tiled( g_selected_monitor->client_list ); client && next_tiled( client->next_client ); client = next_tiled( client->next_client ) ) {}
                if ( client ) {
                        detach( client );
                        attach( client );
                        detach_stack( client );
                        attach_stack( client );
                }
        } else {
                if ( ( client = next_tiled( g_selected_monitor->client_list ) ) ) {
                        detach( client );
                        enqueue( client );
                        detach_stack( client );
                        enqueue_stack( client );
                }
        }

        if ( client ) {
                arrange( g_selected_monitor );
                //unfocus( focus_client, 1 );
                focus( focus_client );
                restack( g_selected_monitor );
        }
}

void send_to_monitor( Client_t *client, Monitor_t *monitor ) {
        if ( client->monitor == monitor ) return;

        unfocus( client, 1 );
        detach( client );
        detach_stack( client );

        client->monitor = monitor;
        client->tags = monitor->tag_set[ monitor->selected_tags ]; // assign tags of target monitor
        client->x = client->monitor->monitor_x + ( client->monitor->monitor_width - WIDTH( client ) ) / 2;
        client->y = client->monitor->monitor_y + ( client->monitor->monitor_height - HEIGHT( client ) ) / 2;

        if ( !strcmp( client->name, g_master_config.window_config.scratchpad_name ) ) {
                client->monitor->tag_set[ client->monitor->selected_tags ] |= client->tags = g_scratchpad_tag;
        }

        attach( client );
        attach_stack( client );
        set_client_tag_property( client );
        focus( NULL );
        arrange( NULL );
}

// TODO: This whole function needs some work to make it easier to read. Its a logical nightmare to understand at a glance.
// This will likely just need to be a consolidated fullscreen state, instead of the current `fake_fullscreen_state` + `is_fullscreen`.
void set_fullscreen( Client_t *client, bool set_fullscreen ) {
        XEvent event;
        int save_state = 0, restore_state = 0, restore_fake_fullscreen = 0;

        if ( ( set_fullscreen && client->fake_fullscreen_state == NO_FAKE_FULLSCREEN && !client->is_fullscreen ) // no normal fullscreen + no fake fullscreen
             || ( set_fullscreen && client->fake_fullscreen_state == FAKE_FULLSCREEN_AND_NORMAL_FULLSCREEN ) )   // normal fullscreen + fake fullscreen
                save_state = 1;                                                                                  // go actual fullscreen

        else if ( ( !set_fullscreen && client->fake_fullscreen_state == NO_FAKE_FULLSCREEN && client->is_fullscreen ) // normal fullscreen + no fake fullscreen
                  || ( !set_fullscreen && client->fake_fullscreen_state >= FAKE_FULLSCREEN_AND_NORMAL_FULLSCREEN ) )  // normal fullscreen + fake fullscreen
                restore_state = 1;                                                                                    // go back into tiled

        // If leaving fullscreen and the window was previously fake fullscreen (2), then restore
        // that while staying in fullscreen. The exception to this is if we are in said state, but
        // the client itself disables fullscreen (3) then we let the client go out of fullscreen
        // while keeping fake fullscreen enabled (as otherwise there will be a mismatch between the
        // client and the window manager's perception of the client's fullscreen state).
        if ( client->fake_fullscreen_state == FAKE_FULLSCREEN_AND_NORMAL_FULLSCREEN && !set_fullscreen && client->is_fullscreen ) {
                restore_fake_fullscreen = 1;
                client->is_fullscreen = true;
                set_fullscreen = 1;
        }

        if ( set_fullscreen != client->is_fullscreen ) {
                // Only send property change if necessary
                if ( set_fullscreen ) {
                        XChangeProperty( g_display, client->window, g_net_atoms[ NET_WM_STATE ], XA_ATOM, 32, PropModeReplace, (unsigned char *) &g_net_atoms[ NET_WM_STATE_FULLSCREEN ], 1 );
                } else {
                        XChangeProperty( g_display, client->window, g_net_atoms[ NET_WM_STATE ], XA_ATOM, 32, PropModeReplace, 0, 0 );
                }
        }

        client->is_fullscreen = set_fullscreen;

        // Some clients, e.g. firefox, will send a client message informing the window manager
        // that it is going into fullscreen after receiving the above signal. This has the side
        // effect of this function (set_fullscreen) sometimes being called twice when toggling
        // fullscreen on and off via the window manager as opposed to the application itself.
        // To protect against obscure issues where the client settings are stored or restored
        // when they are not supposed to we add a bit-lock on the old state so that
        // settings can only be stored and restored in that precise order.
        if ( save_state && !( client->old_state & ( 1 << 1 ) ) ) {

                client->old_border_width = client->border_width;
                client->old_state = client->is_floating | ( 1 << 1 );

                if ( client->is_floating ) {
                        client->old_floating_x = client->x;
                        client->old_floating_y = client->y;
                        client->old_floating_width = client->width;
                        client->old_floating_height = client->height;
                }

                client->border_width = 0;
                client->is_floating = true;

                resize_client( client, client->monitor->monitor_x, client->monitor->monitor_y, client->monitor->monitor_width, client->monitor->monitor_height );
                XRaiseWindow( g_display, client->window );

        } else if ( restore_state && ( client->old_state & ( 1 << 1 ) ) ) {

                client->border_width = client->old_border_width;
                client->is_floating = ( client->old_state = ( client->old_state & 1 ) );

                if ( restore_fake_fullscreen || client->fake_fullscreen_state == RESTORE_FAKE_FULLSCREEN ) {
                        client->fake_fullscreen_state = ONLY_FAKE_FULLSCREEN;
                }

                // The client may have been moved to another monitor whilst in fullscreen which, if tiled,
                // we address by doing a full arrange of tiled clients. If the client is floating then the
                // height and width may be larger than the monitor's window area, so we cap that by
                // ensuring max / min values.
                if ( client->is_floating ) {
                        const int border_difference = ( g_master_config.window_config.floating_border_width - g_master_config.window_config.normal_border_width ) * 2;
                        resize( client, client->old_floating_x, client->old_floating_y, client->old_floating_width - border_difference, client->old_floating_height - border_difference, 0 );
                        restack( client->monitor );
                } else {
                        arrange( client->monitor );
                }
        } else {
                resize_client( client, client->x, client->y, client->width, client->height );
        }

        // Exception: if the client was in actual fullscreen, and we exit out to fake fullscreen
        // mode, then the focus would sometimes drift to whichever window is under the mouse cursor
        // at the time. To avoid this we ask X for all EnterNotify events and just ignore them.
        if ( !client->is_fullscreen ) {
                while ( XCheckMaskEvent( g_display, EnterWindowMask, &event ) ) {}
        }
}

void set_sticky( Client_t *client, const bool sticky ) {
        client->is_sticky = sticky;
        if ( client->is_sticky ) {
                XChangeProperty( g_display, client->window, g_net_atoms[ NET_WM_STATE ], XA_ATOM, 32, PropModeReplace, (unsigned char *) &g_net_atoms[ NET_WM_STATE_STICKY ], 1 );
        } else {
                XChangeProperty( g_display, client->window, g_net_atoms[ NET_WM_STATE ], XA_ATOM, 32, PropModeReplace, 0, 0 );
                arrange( client->monitor );
        }
}

void set_urgent( Client_t *client, const bool urgent ) {
        XWMHints *hints;
        client->is_urgent = urgent;
        if ( !( ( hints = XGetWMHints( g_display, client->window ) ) ) ) return;
        hints->flags = urgent ? ( hints->flags | XUrgencyHint ) : ( hints->flags & ~XUrgencyHint );
        XSetWMHints( g_display, client->window, hints );
        XFree( hints );
}

void show_or_hide( Client_t *client ) {
        if ( !client ) return;

        if ( ISVISIBLE( client ) ) {
                // Show clients top down
                XMoveWindow( g_display, client->window, client->x, client->y );

                if ( ( !client->monitor->tags[ client->monitor->current_tag_index ].layout->arrange || client->is_floating ) && !client->is_fullscreen ) {
                        resize( client, client->x, client->y, client->width, client->height, 0 );
                }

                show_or_hide( client->stacking_next_client );
        } else {
                // Else hide bottom up
                show_or_hide( client->stacking_next_client );
                XMoveWindow( g_display, client->window, WIDTH( client ) * -2, client->y );
        }
}

void toggle_fake_fullscreen( const Argument_t *argument ) {
        Client_t *client = g_selected_monitor->selected_client;

        if ( !client ) return;

        if ( client->fake_fullscreen_state != ONLY_FAKE_FULLSCREEN && client->is_fullscreen ) {
                // Exit fullscreen --> fake fullscreen
                client->fake_fullscreen_state = FAKE_FULLSCREEN_AND_NORMAL_FULLSCREEN;
                set_fullscreen( client, false );
        } else if ( client->fake_fullscreen_state == ONLY_FAKE_FULLSCREEN ) {
                // Exit fake fullscreen
                set_fullscreen( client, false );
                client->fake_fullscreen_state = NO_FAKE_FULLSCREEN;
        } else {
                // Enter fake fullscreen
                client->fake_fullscreen_state = ONLY_FAKE_FULLSCREEN;
                set_fullscreen( client, true );
        }
}

void toggle_floating( const Argument_t *Argument ) {
        Client_t *client = g_selected_monitor->selected_client;

        if ( !client ) return;

        // no support for fullscreen windows
        if ( client->is_fullscreen && client->fake_fullscreen_state != 1 ) return;

        client->is_floating = !client->is_floating || client->is_fixed;
        if ( client->is_floating ) {
                client->border_width = g_master_config.window_config.floating_border_width;

                XSetWindowBorder( g_display, client->window, THEME.color_client_selected[ FLOATING_BORDER ].pixel );
                configure( client );

                // TODO: There is a pretty significant loss of window size here that should be looked at.
                // Just spam this function on a window and you will see it pretty rapidly lose size.
                const int border_difference = ( g_master_config.window_config.floating_border_width - g_master_config.window_config.normal_border_width ) * 2;
                resize( client, client->old_floating_x, client->old_floating_y, client->old_floating_width - border_difference, client->old_floating_height - border_difference, 0 );
        } else {
                client->old_floating_x = client->x;
                client->old_floating_y = client->y;
                client->old_floating_width = client->width;
                client->old_floating_height = client->height;
                client->border_width = g_master_config.window_config.normal_border_width;

                XSetWindowBorder( g_display, client->window, THEME.color_client_selected[ NORMAL_BORDER ].pixel );
                configure( client );
        }

        arrange( client->monitor );
}

void toggle_fullscreen( const Argument_t *argument ) {
        Client_t *client = g_selected_monitor->selected_client;

        if ( !client ) return;

        if ( client->fake_fullscreen_state == ONLY_FAKE_FULLSCREEN ) {
                // fake fullscreen --> fullscreen
                client->fake_fullscreen_state = FAKE_FULLSCREEN_AND_NORMAL_FULLSCREEN;
                set_fullscreen( client, true );
        } else {
                set_fullscreen( client, !client->is_fullscreen );
        }
}

// TODO: This patch is bugged. Sometimes it picks up client's it shouldn't. It needs to be re-written.
void toggle_scratch( const Argument_t *argument ) {
        Client_t *client;
        unsigned int found = 0;

        for ( client = g_selected_monitor->client_list; client && !( ( found = client->tags & g_scratchpad_tag ) ); client = client->next_client ) {}

        if ( found ) {
                const unsigned int new_tag_set = CURRENT_SELECTED_TAGS ^ g_scratchpad_tag;

                if ( new_tag_set ) {
                        g_selected_monitor->tag_set[ g_selected_monitor->selected_tags ] = new_tag_set;
                        focus( NULL );
                        arrange( g_selected_monitor );
                }

                if ( ISVISIBLE( client ) ) {
                        focus( client );
                        restack( g_selected_monitor );
                        XWarpPointer( g_display, None, client->window, 0, 0, 0, 0, WIDTH( client ) / 2, HEIGHT( client ) / 2 );
                }
        } else {
                spawn( argument );
        }
}

void toggle_sticky( const Argument_t *argument ) {
        if ( !g_selected_monitor->selected_client ) return;
        set_sticky( g_selected_monitor->selected_client, !g_selected_monitor->selected_client->is_sticky );
        arrange( g_selected_monitor );
}

void unfocus( Client_t *client, const bool set_focus ) {
        if ( !client ) return;
        grab_buttons( client, false );
        g_last_focused_client = client;
        if ( set_focus ) {
                XSetInputFocus( g_display, g_root_window, RevertToPointerRoot, CurrentTime );
                XDeleteProperty( g_display, g_root_window, g_net_atoms[ NET_ACTIVE_WINDOW ] );
        }
}

void unmanage( Client_t *client, const bool destroyed ) {
        Monitor_t *monitor = client->monitor;

        detach( client );
        detach_stack( client );
        free_icon( client );

        if ( !destroyed ) {
                XWindowChanges window_changes = { .border_width = client->old_border_width };

                XGrabServer( g_display ); // Avoid race conditions
                XSetErrorHandler( x_error_dummy );
                XSelectInput( g_display, client->window, NoEventMask );
                XConfigureWindow( g_display, client->window, CWBorderWidth, &window_changes ); // Restore border
                XUngrabButton( g_display, AnyButton, AnyModifier, client->window );
                set_client_state( client, WithdrawnState );
                XSync( g_display, False );
                XSetErrorHandler( x_error );
                XUngrabServer( g_display );
        }

        if ( g_last_focused_client == client ) {
                g_last_focused_client = NULL;
        }

        free( client );
        focus( NULL );
        update_client_list();
        arrange( monitor );

        if ( monitor == g_selected_monitor && monitor->selected_client ) {
                XWarpPointer( g_display, None, monitor->selected_client->window, 0, 0, 0, 0, monitor->selected_client->width / 2, monitor->selected_client->height / 2 );
        }
}

void update_icon( Client_t *client ) {
        free_icon( client );
        client->icon = get_icon_properties( client->window, &client->icon_width, &client->icon_height );
}

void update_size_hints( Client_t *client ) {
        long dummy;
        XSizeHints size_hints;

        // size_hints is uninitialized, ensure that size_hints.flags aren't used
        if ( !XGetWMNormalHints( g_display, client->window, &size_hints, &dummy ) ) {
                size_hints.flags = PSize;
        }

        if ( size_hints.flags & PBaseSize ) {
                client->base_width = size_hints.base_width;
                client->base_height = size_hints.base_height;
        } else if ( size_hints.flags & PMinSize ) {
                client->base_width = size_hints.min_width;
                client->base_height = size_hints.min_height;
        } else {
                client->base_width = client->base_height = 0;
        }

        if ( size_hints.flags & PResizeInc ) {
                client->increment_width = size_hints.width_inc;
                client->increment_height = size_hints.height_inc;
        } else {
                client->increment_width = client->increment_height = 0;
        }

        if ( size_hints.flags & PMaxSize ) {
                client->max_width = size_hints.max_width;
                client->max_height = size_hints.max_height;
        } else {
                client->max_width = client->max_height = 0;
        }

        if ( size_hints.flags & PMinSize ) {
                client->min_width = size_hints.min_width;
                client->min_height = size_hints.min_height;
        } else if ( size_hints.flags & PBaseSize ) {
                client->min_width = size_hints.base_width;
                client->min_height = size_hints.base_height;
        } else {
                client->min_width = client->min_height = 0;
        }

        if ( size_hints.flags & PAspect ) {
                client->min_aspect_ratio = (double) size_hints.min_aspect.y / (double) size_hints.min_aspect.x;
                client->max_aspect_ratio = (double) size_hints.max_aspect.x / (double) size_hints.max_aspect.y;
        } else {
                client->max_aspect_ratio = client->min_aspect_ratio = 0.0;
        }

        client->is_fixed = ( client->max_width && client->max_height && client->max_width == client->min_width && client->max_height == client->min_height );
        client->should_refresh_size_hints = false;
}

void update_title( Client_t *client ) {
        if ( !get_text_properties( client->window, g_net_atoms[ NET_WM_NAME ], client->name, sizeof client->name ) ) {
                get_text_properties( client->window, XA_WM_NAME, client->name, sizeof client->name );
        }

        // Hack to mark broken clients
        if ( client->name[ 0 ] == '\0' ) {
                strcpy( client->name, BROKEN );
        }
}

void update_window_manager_hints( Client_t *client ) {
        XWMHints *hints;

        if ( ( hints = XGetWMHints( g_display, client->window ) ) ) {
                if ( client == g_selected_monitor->selected_client && hints->flags & XUrgencyHint ) {
                        hints->flags &= ~XUrgencyHint;
                        XSetWMHints( g_display, client->window, hints );
                } else {
                        client->is_urgent = ( hints->flags & XUrgencyHint ) ? true : false;
                }

                if ( hints->flags & InputHint ) {
                        client->should_never_focus = !hints->input;
                } else {
                        client->should_never_focus = false;
                }

                XFree( hints );
        }
}

void update_window_type( Client_t *client ) {

        const Atom state = get_atom_properties( client, g_net_atoms[ NET_WM_STATE ] );
        const Atom window_type = get_atom_properties( client, g_net_atoms[ NET_WM_WINDOW_TYPE ] );

        if ( state == g_net_atoms[ NET_WM_STATE_FULLSCREEN ] ) {
                set_fullscreen( client, true );
        }

        if ( state == g_net_atoms[ NET_WM_STATE_STICKY ] ) {
                set_sticky( client, 1 );
        }

        // TODO: Maybe also unmanage?
        if ( window_type == g_net_atoms[ NET_WM_WINDOW_TYPE_DIALOG ] ) {
                client->is_floating = true;
        }
}

Client_t *window_to_client( const Window window ) {
        for ( const Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                for ( Client_t *client = monitor->client_list; client; client = client->next_client ) {
                        if ( client->window == window ) {
                                return client;
                        }
                }
        }

        return NULL;
}

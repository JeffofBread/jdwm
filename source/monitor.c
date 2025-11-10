#include "monitor.h"

#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include "bar.h"
#include "client.h"
#include "core.h"
#include "layout.h"
#include "parser.h"
#include "util.h"
#include "xutil.h"

Monitor_t *g_monitors, *g_selected_monitor, *g_last_selected_monitor;

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>

// TODO: Clean up variable names
static int is_unique_geometry( const XineramaScreenInfo *unique, size_t n, const XineramaScreenInfo *info ) {
        while ( n-- ) {
                if ( unique[ n ].x_org == info->x_org && unique[ n ].y_org == info->y_org && unique[ n ].width == info->width && unique[ n ].height == info->height ) {
                        return 0;
                }
        }
        return 1;
}
#endif /* XINERAMA */

void arrange( Monitor_t *monitor ) {
        if ( monitor ) {
                show_or_hide( monitor->client_stack );
        } else {
                for ( monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                        show_or_hide( monitor->client_stack );
                }
        }

        if ( monitor ) {
                arrange_monitor( monitor );
                restack( monitor );
        } else {
                for ( monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                        arrange_monitor( monitor );
                }
        }
}

void arrange_monitor( Monitor_t *monitor ) {
        memcpy( monitor->tags[ monitor->current_tag_index ].layout_symbol, monitor->tags[ monitor->current_tag_index ].layout->symbol,
                sizeof( monitor->tags[ monitor->current_tag_index ].layout_symbol ) );

        if ( monitor->tags[ monitor->current_tag_index ].layout->arrange ) {
                monitor->tags[ monitor->current_tag_index ].layout->arrange( monitor );
        }
}

void cleanup_monitor( Monitor_t *monitor ) {

        if ( monitor == g_monitors ) {
                g_monitors = g_monitors->next_monitor;
        } else {
                Monitor_t *tmp_monitor;
                for ( tmp_monitor = g_monitors; tmp_monitor && tmp_monitor->next_monitor != monitor; tmp_monitor = tmp_monitor->next_monitor ) {}
                tmp_monitor->next_monitor = monitor->next_monitor;
        }

        XUnmapWindow( g_display, monitor->bar_window );
        XDestroyWindow( g_display, monitor->bar_window );
        free( monitor );
}

Monitor_t *create_monitor( void ) {
        Monitor_t *monitor = ecalloc( 1, sizeof( Monitor_t ) );

        strncpy( monitor->tags[ monitor->current_tag_index ].layout_symbol, LAYOUTS[ 0 ].symbol, sizeof( monitor->tags[ monitor->current_tag_index ].layout_symbol ) );
        monitor->tag_set[ 0 ] = monitor->tag_set[ 1 ] = 1;

        for ( int i = 0; i < TAGS_COUNT; i++ ) {

                // Layout
                monitor->tags[ i ].layout = &LAYOUTS[ 0 ];
                monitor->tags[ i ].master_factor = ( g_master_config.layout_config.default_master_factor ) / 100.0;
                monitor->tags[ i ].number_of_clients_in_master_area = g_master_config.layout_config.default_number_of_clients_in_master_area;

                // Bar
                monitor->tags[ i ].show_bar = g_master_config.bar_config.show_by_default;
                monitor->tags[ i ].top_bar = g_master_config.bar_config.enable_top_bar;

                // Gaps
                monitor->tags[ i ].enable_gaps = 1;
                monitor->tags[ i ].inner_horizontal_gap = g_master_config.layout_config.default_inner_horizontal_gap;
                monitor->tags[ i ].inner_vertical_gap = g_master_config.layout_config.default_inner_vertical_gap;
                monitor->tags[ i ].outer_horizontal_gap = g_master_config.layout_config.default_outer_horizontal_gap;
                monitor->tags[ i ].outer_vertical_gap = g_master_config.layout_config.default_outer_vertical_gap;
        }

        return monitor;
}

Monitor_t *direction_to_next_monitor( const int direction ) {
        Monitor_t *monitor = NULL;

        if ( direction > 0 ) {
                if ( !( ( monitor = g_selected_monitor->next_monitor ) ) ) {
                        monitor = g_monitors;
                }
        } else if ( g_selected_monitor == g_monitors ) {
                for ( monitor = g_monitors; monitor->next_monitor; monitor = monitor->next_monitor ) {}
        } else {
                for ( monitor = g_monitors; monitor->next_monitor != g_selected_monitor; monitor = monitor->next_monitor ) {}
        }

        return monitor;
}

void focus_monitor( const Argument_t *argument ) {
        if ( !g_monitors->next_monitor ) return;

        Monitor_t *monitor;
        if ( ( monitor = direction_to_next_monitor( argument->i ) ) == g_selected_monitor ) return;

        unfocus( g_selected_monitor->selected_client, 0 );
        g_selected_monitor = monitor;
        focus( NULL );

        // Thank you to u/bakkeby on r/dwm for the solution, which was derived from the cursor warp patch
        if ( g_selected_monitor->selected_client ) {
                XWarpPointer( g_display, None, g_selected_monitor->selected_client->window, 0, 0, 0, 0, g_selected_monitor->selected_client->width / 2,
                              g_selected_monitor->selected_client->height / 2 );
        } else {
                XWarpPointer( g_display, None, g_root_window, 0, 0, 0, 0, g_selected_monitor->window_x + g_selected_monitor->window_width / 2,
                              g_selected_monitor->window_y + g_selected_monitor->window_height / 2 );
        }
}

// TODO: Maybe look at renaming `tmp_master_factor` and `tmp_stack_factor`.
void get_facts( const Monitor_t *monitor, const int master_size, const int stack_size, double *master_factor, double *stack_factor, int *master_remainder, int *stack_remainder ) {
        int i;
        Client_t *client;

        for ( i = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), i++ ) {}

        const int tmp_master_factor = MIN( i, monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area );
        const int tmp_stack_factor = i - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area;

        int master_total = 0, stack_total = 0;
        for ( i = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), i++ ) {
                if ( i < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                        master_total += master_size / tmp_master_factor;
                } else {
                        stack_total += stack_size / tmp_stack_factor;
                }
        }

        *master_factor = tmp_master_factor;             // total factor of master area
        *stack_factor = tmp_stack_factor;               // total factor of stack area
        *master_remainder = master_size - master_total; // the remainder (rest) of pixels after an even master split
        *stack_remainder = stack_size - stack_total;    // the remainder (rest) of pixels after an even stack split
}

Monitor_t *rectangle_to_monitor( const int x, const int y, const int width, const int height ) {

        int greatest_area = 0;
        for ( Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                const int current_monitor_area = (
                        MAX( 0, MIN( ( x ) + ( width ), ( monitor )->window_x + ( monitor )->window_width ) - MAX( ( x ), ( monitor )->window_x ) ) *
                        MAX( 0, MIN( ( y ) + ( height ), ( monitor )->window_y + ( monitor )->window_height ) - MAX( ( y ), ( monitor )->window_y ) ) );

                if ( current_monitor_area > greatest_area ) {
                        greatest_area = current_monitor_area;
                        g_selected_monitor = monitor;
                }
        }

        return g_selected_monitor;
}

void restack( Monitor_t *monitor ) {
        draw_bar( monitor );

        if ( !monitor->selected_client ) return;

        if ( monitor->selected_client->is_floating || !monitor->tags[ monitor->current_tag_index ].layout->arrange ) {
                XRaiseWindow( g_display, monitor->selected_client->window );
        }

        XWindowChanges window_change;
        if ( monitor->tags[ monitor->current_tag_index ].layout->arrange ) {
                window_change.stack_mode = Below;
                window_change.sibling = monitor->bar_window;

                for ( const Client_t *client = monitor->client_stack; client; client = client->stacking_next_client ) {
                        if ( !client->is_floating && ISVISIBLE( client ) ) {
                                XConfigureWindow( g_display, client->window, CWSibling | CWStackMode, &window_change );
                                window_change.sibling = client->window;
                        }
                }
        }

        XSync( g_display, False );

        XEvent event;
        while ( XCheckMaskEvent( g_display, EnterWindowMask, &event ) );
}

void set_master_factor( const Argument_t *argument ) {
        if ( !argument || !CURRENT_SELECTED_TAG_INDEX.layout->arrange ) return;

        const float new_master_factor = argument->f < 1.0f ? argument->f + (float) CURRENT_SELECTED_TAG_INDEX.master_factor : argument->f - 1.0f;

        if ( new_master_factor < 0.05 || new_master_factor > 0.95 ) return;

        CURRENT_SELECTED_TAG_INDEX.master_factor = new_master_factor;
        arrange( g_selected_monitor );
}

// TODO: Cleanup
int update_geometry( void ) {
        int dirty = 0;

        #ifdef XINERAMA
        if ( XineramaIsActive( g_display ) ) {
                int i, j, n, nn;
                Client_t *c;
                Monitor_t *m;
                XineramaScreenInfo *info = XineramaQueryScreens( g_display, &nn );
                XineramaScreenInfo *unique = NULL;

                for ( n = 0, m = g_monitors; m; m = m->next_monitor, n++ ) {}
                /* only consider unique geometries as separate screens */
                unique = ecalloc( nn, sizeof( XineramaScreenInfo ) );
                for ( i = 0, j = 0; i < nn; i++ ) if ( is_unique_geometry( unique, j, &info[ i ] ) ) memcpy( &unique[ j++ ], &info[ i ], sizeof( XineramaScreenInfo ) );
                XFree( info );
                nn = j;

                /* new monitors if nn > n */
                for ( i = n; i < nn; i++ ) {
                        for ( m = g_monitors; m && m->next_monitor; m = m->next_monitor ) {}
                        if ( m ) m->next_monitor = create_monitor();
                        else g_monitors = create_monitor();
                }
                for ( i = 0, m = g_monitors; i < nn && m; m = m->next_monitor, i++ )
                        if ( i >= n || unique[ i ].x_org != m->monitor_x || unique[ i ].y_org != m->monitor_y || unique[ i ].width != m->monitor_width || unique[ i ].height != m->monitor_height ) {
                                dirty = 1;
                                m->monitor_number = i;
                                m->monitor_x = m->window_x = unique[ i ].x_org;
                                m->monitor_y = m->window_y = unique[ i ].y_org;
                                m->monitor_width = m->window_width = unique[ i ].width;
                                m->monitor_height = m->window_height = unique[ i ].height;
                                update_bar_position( m );
                        }
                /* removed monitors if n > nn */
                for ( i = nn; i < n; i++ ) {
                        for ( m = g_monitors; m && m->next_monitor; m = m->next_monitor ) {}
                        while ( ( c = m->client_list ) ) {
                                dirty = 1;
                                m->client_list = c->next_client;
                                detach_stack( c );
                                c->monitor = g_monitors;
                                attach( c );
                                attach_stack( c );
                        }
                        if ( m == g_selected_monitor ) g_selected_monitor = g_monitors;
                        cleanup_monitor( m );
                }
                free( unique );
        } else
        #endif /* XINERAMA */
        {
                /* default monitor setup */
                if ( !g_monitors ) g_monitors = create_monitor();
                if ( g_monitors->monitor_width != g_screen_width || g_monitors->monitor_height != g_screen_height ) {
                        dirty = 1;
                        g_monitors->monitor_width = g_monitors->window_width = g_screen_width;
                        g_monitors->monitor_height = g_monitors->window_height = g_screen_height;
                        update_bar_position( g_monitors );
                }
        }
        if ( dirty ) {
                g_selected_monitor = g_monitors;
                g_selected_monitor = window_to_monitor( g_root_window );
        }
        return dirty;
}

Monitor_t *window_to_monitor( const Window window ) {

        int x, y;
        if ( window == g_root_window && get_root_pointer( &x, &y ) ) {
                return rectangle_to_monitor( x, y, 1, 1 );
        }

        for ( Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                if ( window == monitor->bar_window ) {
                        return monitor;
                }
        }

        Client_t *client;
        if ( ( client = window_to_client( window ) ) ) {
                return client->monitor;
        }

        return g_selected_monitor;
}

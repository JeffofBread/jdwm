#include "bar.h"

#include <string.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "client.h"
#include "common.h"
#include "core.h"
#include "draw.h"
#include "monitor.h"
#include "parser.h"
#include "systray.h"
#include "tags.h"
#include "xutil.h"

static const XPoint STICKY_ICON_BOUNDING_BOX = { 4, 8 };
static const XPoint STICKY_ICON_VERTICES[ ] = { { 0, 0 }, { 4, 0 }, { 4, 8 }, { 2, 6 }, { 0, 8 }, { 0, 0 } };

int g_bar_height;             /* bar height */
int g_vertical_bar_padding;   /* vertical padding for bar */
int g_horizontal_bar_padding; /* side padding for bar */

char g_status_text[ 128 ];

// TODO: Probably needs to be broken down into smaller helper functions
void draw_bar( Monitor_t *monitor ) {
        unsigned int bar_width = 0, text_width = 0, systray_width = 0;
        unsigned int occupied_tags = 0, urgent_tags = 0;

        // 9, 6, and 2 are magic numbers that just control the size/spacing
        // of the floating indicator (I think). I'm not sure their rhyme or
        // reason but im not touching them.
        const unsigned int box_spacing = g_drawable->fonts->h / 9;
        const unsigned int box_width = g_drawable->fonts->h / 6 + 2;

        if ( !monitor->tags[ monitor->current_tag_index ].show_bar ) return;

        if ( g_master_config.bar_config.show_system_tray ) {
                if ( monitor == systray_to_monitor( monitor ) ) systray_width = get_systray_width();
        }

        // Draw status first so it can be overdrawn by tags later.
        // Status is only drawn if the toggle is enabled and if the
        // bar we are drawing is on the currently selected monitor.
        if ( g_master_config.bar_config.show_status_text && monitor == g_selected_monitor ) {
                drawable_set_scheme( g_drawable, THEME.color_statusbar );
                text_width = TEXT_WIDTH( g_status_text ) - g_left_right_text_padding / 2;
                draw_text( g_drawable, monitor->window_width - (int) text_width - (int) systray_width - 2 * g_horizontal_bar_padding, 0, text_width, g_bar_height, g_left_right_text_padding / 2,
                           g_status_text, 0 );
        }

        resize_bar_window( monitor );

        // Save all the occupied and urgent tags to we can render them differently later
        for ( const Client_t *client = monitor->client_list; client; client = client->next_client ) {
                occupied_tags |= client->tags;
                if ( g_master_config.bar_config.show_tags && client->is_urgent ) urgent_tags |= client->tags;
        }

        for ( int i = 0; i < TAGS_COUNT; i++ ) {

                // Do not draw vacant tags
                if ( !( occupied_tags & 1 << i || monitor->tag_set[ monitor->selected_tags ] & 1 << i ) ) continue;

                if ( g_master_config.bar_config.show_tags ) {
                        const int tag_width = TEXT_WIDTH( g_master_config.bar_config.tags[ i ] );

                        drawable_set_scheme( g_drawable, monitor->tag_set[ monitor->selected_tags ] & 1 << i ? THEME.color_tag_selected : THEME.color_tag_unselected );

                        draw_text( g_drawable, (int) bar_width, 0, tag_width, g_bar_height, g_left_right_text_padding / 2, g_master_config.bar_config.tags[ i ], (int) urgent_tags & 1 << i );

                        if ( g_master_config.bar_config.show_tag_underline ) {
                                //@formatter:off
                                if ( monitor->tag_set[ monitor->selected_tags ] & 1 << i ) {
                                        draw_rectangle(
                                                g_drawable,
                                                (int) bar_width + g_master_config.bar_config.underline_horizontal_padding,
                                                g_bar_height - g_master_config.bar_config.underline_thickness - g_master_config.bar_config.underline_vertical_padding,
                                                ( g_master_config.bar_config.underline_horizontal_padding * 2 ) > tag_width ? 0 : tag_width - ( g_master_config.bar_config.underline_horizontal_padding * 2 ),
                                                g_master_config.bar_config.underline_thickness,
                                                1,
                                                0 );
                                }
                                //@formatter:on
                        }

                        if ( g_master_config.bar_config.show_floating_indicator && occupied_tags & 1 << i ) {
                                draw_rectangle( 0, 0, 0, 0, 0, monitor == g_selected_monitor && g_selected_monitor->selected_client && g_selected_monitor->selected_client->tags & 1 << i,
                                                (int) urgent_tags & 1 << i );
                        }

                        bar_width += tag_width;
                }
        }

        if ( g_master_config.bar_config.show_layout_symbol ) {
                const int layout_symbol_width = TEXT_WIDTH( monitor->tags[ monitor->current_tag_index ].layout_symbol );

                drawable_set_scheme( g_drawable, THEME.color_layout );

                bar_width = draw_text( g_drawable, (int) bar_width, 0, layout_symbol_width, g_bar_height, g_left_right_text_padding / 2, monitor->tags[ monitor->current_tag_index ].layout_symbol, 0 );
        }

        const unsigned int title_width = monitor->window_width - text_width - systray_width - bar_width;
        if ( title_width > (long) g_bar_height ) {
                if ( g_master_config.bar_config.show_client_title && monitor->selected_client ) {

                        // Fix overflow when window name is bigger than window width
                        long middle = ( monitor->window_width - TEXT_WIDTH( monitor->selected_client->name ) ) / 2 - bar_width;

                        // Make sure name will not overlap on tags even when it is very long
                        middle = middle >= g_left_right_text_padding / 2 ? middle : g_left_right_text_padding / 2;

                        drawable_set_scheme( g_drawable, monitor == g_selected_monitor ? THEME.color_titlebar_selected : THEME.color_titlebar_unselected );

                        // TODO: Look at cutting down code by just saving the spacing to variables and running the draw
                        // commands once with these spacing after the if/else
                        if ( !g_master_config.bar_config.enable_centered_client_title ) {

                                draw_text( g_drawable, (int) bar_width, 0, title_width - 2 * g_horizontal_bar_padding, g_bar_height,
                                           g_left_right_text_padding / 2 + ( monitor->selected_client->icon
                                                                                     ? monitor->selected_client->icon_width + g_master_config.bar_config.client_icon_padding
                                                                                     : 0 ), monitor->selected_client->name, 0 );

                                if ( monitor->selected_client->icon ) {
                                        draw_picture( g_drawable, (int) bar_width + g_left_right_text_padding / 2, (int) ( g_bar_height - monitor->selected_client->icon_height ) / 2,
                                                      monitor->selected_client->icon_width, monitor->selected_client->icon_height, monitor->selected_client->icon );
                                }

                                if ( g_master_config.bar_config.show_floating_indicator && monitor->selected_client->is_floating ) {
                                        draw_rectangle( g_drawable, (int) ( bar_width + box_spacing ), (int) box_spacing, box_width, box_width, monitor->selected_client->is_fixed, 0 );
                                }

                                if ( monitor->selected_client->is_sticky ) {
                                        draw_polygon( g_drawable, (int) ( bar_width + box_spacing ), (int) ( monitor->selected_client->is_floating ? box_spacing * 2 + box_width : box_spacing ),
                                                      STICKY_ICON_BOUNDING_BOX.x, STICKY_ICON_BOUNDING_BOX.y, (int) box_width,
                                                      (int) box_width * STICKY_ICON_BOUNDING_BOX.y / STICKY_ICON_BOUNDING_BOX.x, STICKY_ICON_VERTICES, LENGTH( STICKY_ICON_VERTICES ), Nonconvex,
                                                      (int) ( monitor->selected_client->tags & monitor->tag_set[ monitor->selected_tags ] ) );
                                }
                        } else {
                                draw_text( g_drawable, (int) bar_width, 0, title_width - 2 * g_horizontal_bar_padding, g_bar_height,
                                           middle + ( monitor->selected_client->icon ? monitor->selected_client->icon_width + g_master_config.bar_config.client_icon_padding : 0 ),
                                           monitor->selected_client->name, 0 );

                                if ( monitor->selected_client->icon ) {
                                        draw_picture( g_drawable, (int) ( bar_width + middle ), (int) ( g_bar_height - monitor->selected_client->icon_height ) / 2,
                                                      monitor->selected_client->icon_width, monitor->selected_client->icon_height, monitor->selected_client->icon );
                                }

                                if ( g_master_config.bar_config.show_floating_indicator && monitor->selected_client->is_floating ) {
                                        draw_rectangle( g_drawable, (int) ( bar_width + middle - g_master_config.bar_config.client_icon_padding - 3 ), (int) box_spacing, box_width, box_width,
                                                        monitor->selected_client->is_fixed, 0 );
                                }

                                if ( monitor->selected_client->is_sticky ) {
                                        draw_polygon( g_drawable, (int) ( bar_width + middle - g_master_config.bar_config.client_icon_padding - 3 ),
                                                      (int) ( monitor->selected_client->is_floating ? box_spacing * 2 + box_width : box_spacing ), STICKY_ICON_BOUNDING_BOX.x,
                                                      STICKY_ICON_BOUNDING_BOX.y, (int) box_width, (int) box_width * STICKY_ICON_BOUNDING_BOX.y / STICKY_ICON_BOUNDING_BOX.x, STICKY_ICON_VERTICES,
                                                      LENGTH( STICKY_ICON_VERTICES ), Nonconvex, (int) ( monitor->selected_client->tags & monitor->tag_set[ monitor->selected_tags ] ) );
                                }
                        }
                } else {
                        drawable_set_scheme( g_drawable, THEME.color_titlebar_unselected );
                        draw_rectangle( g_drawable, (int) bar_width, 0, title_width - 2 * g_horizontal_bar_padding, g_bar_height, 1, 1 );
                }
        }

        draw_map( g_drawable, monitor->bar_window, 0, 0, monitor->window_width - systray_width, g_bar_height );
}

void draw_bars( void ) {
        for ( Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) draw_bar( monitor );
}

void resize_bar_window( Monitor_t *monitor ) {
        unsigned int bar_width = monitor->window_width - 2 * g_horizontal_bar_padding;

        if ( g_master_config.bar_config.show_system_tray ) {
                if ( monitor == systray_to_monitor( monitor ) ) bar_width -= get_systray_width();
        }

        XMoveResizeWindow( g_display, monitor->bar_window, monitor->window_x + g_horizontal_bar_padding, monitor->bar_y + g_vertical_bar_padding, bar_width, g_bar_height );
}

void toggle_bar( const Argument_t *argument ) {
        CURRENT_SELECTED_TAG_INDEX.show_bar = !CURRENT_SELECTED_TAG_INDEX.show_bar;

        update_bar_position( g_selected_monitor );
        resize_bar_window( g_selected_monitor );

        if ( g_master_config.bar_config.show_system_tray ) {
                XWindowChanges window_changes;

                if ( !CURRENT_SELECTED_TAG_INDEX.show_bar ) {
                        window_changes.y = -g_bar_height;
                } else if ( CURRENT_SELECTED_TAG_INDEX.show_bar ) {
                        window_changes.y = g_vertical_bar_padding;
                        if ( !CURRENT_SELECTED_TAG_INDEX.show_bar ) {
                                window_changes.y = g_selected_monitor->monitor_height - g_bar_height + g_vertical_bar_padding;
                        }
                }

                XConfigureWindow( g_display, g_systray->window, CWY, &window_changes );
                update_systray();
        }

        arrange( g_selected_monitor );
}

void update_bars( void ) {

        XClassHint class_hint = { "jdwm", "jdwm" };
        XSetWindowAttributes window_attributes = { .override_redirect = True, .background_pixmap = ParentRelative, .event_mask = ButtonPressMask | ExposureMask };

        for ( Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                if ( monitor->bar_window ) continue;

                unsigned int bar_width = monitor->window_width - 2 * g_horizontal_bar_padding;

                if ( g_master_config.bar_config.show_system_tray ) {
                        if ( monitor == systray_to_monitor( monitor ) ) {
                                bar_width -= get_systray_width();
                        }
                }

                monitor->bar_window = XCreateWindow( g_display, g_root_window, monitor->window_x + g_horizontal_bar_padding, monitor->bar_y + g_vertical_bar_padding, bar_width, g_bar_height, 0,
                                                     DefaultDepth( g_display, g_screen ), CopyFromParent, DefaultVisual( g_display, g_screen ), CWOverrideRedirect | CWBackPixmap | CWEventMask,
                                                     &window_attributes );

                XDefineCursor( g_display, monitor->bar_window, *g_cursor[ CURSOR_NORMAL ] );

                if ( g_master_config.bar_config.show_system_tray ) {
                        if ( monitor == systray_to_monitor( monitor ) ) {
                                XMapRaised( g_display, g_systray->window );
                        }
                }

                XMapRaised( g_display, monitor->bar_window );
                XSetClassHint( g_display, monitor->bar_window, &class_hint );
        }
}

void update_bar_position( Monitor_t *monitor ) {
        monitor->window_y = monitor->monitor_y;
        monitor->window_height = monitor->monitor_height;

        if ( monitor->tags[ monitor->current_tag_index ].show_bar ) {
                monitor->window_height = monitor->window_height - g_master_config.bar_config.vertical_bar_padding - g_bar_height;
                monitor->bar_y = monitor->tags[ monitor->current_tag_index ].top_bar ? monitor->window_y : monitor->window_y + monitor->window_height + g_master_config.bar_config.vertical_bar_padding;
                monitor->window_y = monitor->tags[ monitor->current_tag_index ].top_bar ? monitor->window_y + g_bar_height + g_vertical_bar_padding : monitor->window_y;
        } else {
                monitor->bar_y = -g_bar_height - g_vertical_bar_padding;
        }
}

void update_status( void ) {

        if ( g_master_config.bar_config.show_status_text && !get_text_properties( g_root_window, XA_WM_NAME, g_status_text, sizeof( g_status_text ) ) ) {
                strcpy( g_status_text, "jdwm-" VERSION );
        }

        draw_bar( g_selected_monitor );
        update_systray();
}

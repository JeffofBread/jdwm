#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdbool.h>
#include <X11/X.h>
#include <X11/extensions/render.h>
#include "common.h"
#include "monitor.h"

#define ISVISIBLE( C ) ( ( C->tags & C->monitor->tag_set[C->monitor->selected_tags] ) || C->is_sticky )

enum Fake_Fullscreen_State {
        NO_FAKE_FULLSCREEN,
        ONLY_FAKE_FULLSCREEN,
        FAKE_FULLSCREEN_AND_NORMAL_FULLSCREEN,
        RESTORE_FAKE_FULLSCREEN,
};

typedef struct Client {
        char name[ 128 ];
        unsigned int tags;
        double min_aspect_ratio, max_aspect_ratio;
        int x, y, width, height;
        int old_x, old_y, old_width, old_height;
        int old_floating_x, old_floating_y, old_floating_width, old_floating_height;
        int max_width, max_height, min_width, min_height;
        unsigned int icon_width, icon_height;
        int base_width, base_height;
        int increment_width, increment_height;
        int border_width, old_border_width;
        bool is_fixed, is_floating, is_urgent, is_fullscreen, is_sticky, is_steam;
        bool should_ignore_transient, should_never_focus, should_refresh_size_hints;
        int old_state, fake_fullscreen_state;
        struct Client *next_client, *stacking_next_client;
        struct Monitor *monitor;
        Window window;
        Picture icon;
} Client_t;

extern void apply_rules( Client_t *client );
extern int apply_size_hints( Client_t *client, int *x, int *y, int *width, int *height, int interact );
extern void attach( Client_t *client );
extern void attach_stack( Client_t *client );
extern void center_floating( const Argument_t *argument );
extern void client_to_monitor( const Argument_t *argument );
extern void client_and_view_to_tag( const Argument_t *argument );
extern void client_and_view_to_tag_bitmask( const Argument_t *argument );
extern void client_to_tag( const Argument_t *argument );
extern void client_to_tag_bitmask( const Argument_t *argument );
extern void configure( const Client_t *client );
extern void detach( const Client_t *client );
extern void detach_stack( const Client_t *client );
extern void enqueue( Client_t *client );
extern void enqueue_stack( Client_t *client );
extern void float_and_move( const Argument_t *argument );
extern void focus( Client_t *client );
extern void focus_stack( const Argument_t *argument );
extern void free_icon( Client_t *client );
extern void fullscreen_check( Client_t *client );
extern void kill_client( const Argument_t *argument );
extern void lose_fullscreen( Client_t *next_client );
extern void move_mouse( const Argument_t *argument );
extern Client_t *next_tiled( Client_t *client );
extern void pop( Client_t *client );
extern void resize( Client_t *client, int x, int y, int w, int h, int interact );
extern void resize_client( Client_t *client, int x, int y, int w, int h );
extern void resize_mouse( const Argument_t *argument );
extern void rotate_stack( const Argument_t *argument );
extern void send_to_monitor( Client_t *client, Monitor_t *monitor );
extern void set_fullscreen( Client_t *client, bool set_fullscreen );
extern void set_sticky( Client_t *client, bool sticky );
extern void set_urgent( Client_t *client, bool urgent );
extern void show_or_hide( Client_t *client );
extern void toggle_fake_fullscreen( const Argument_t *argument );
extern void toggle_floating( const Argument_t *Argument );
extern void toggle_fullscreen( const Argument_t *argument );
extern void toggle_scratch( const Argument_t *argument );
extern void toggle_sticky( const Argument_t *argument );
extern void unfocus( Client_t *client, bool set_focus );
extern void unmanage( Client_t *client, bool destroyed );
extern void update_icon( Client_t *client );
extern void update_size_hints( Client_t *client );
extern void update_title( Client_t *client );
extern void update_window_manager_hints( Client_t *client );
extern void update_window_type( Client_t *client );
extern Client_t *window_to_client( Window window );

extern const char BROKEN[ ];
extern Client_t *g_last_focused_client;

#endif

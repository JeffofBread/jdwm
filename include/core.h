#ifndef CORE_H_
#define CORE_H_

#include <stdbool.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "common.h"
#include "draw.h"

#define THEME g_master_config.themes[ g_theme_index ]

extern void check_for_other_window_manager( void );
extern void cleanup( void );
extern void increment_theme( const Argument_t *argument );
extern void manage( Window window, const XWindowAttributes *window_attributes );
extern void quit( const Argument_t *argument );
extern void restart( const Argument_t *argument );
extern void run( void );
extern void scan( void );
extern void setup( void );
extern void sigchld( int unused );
extern void sighup( int unused );
extern void sigterm( int unused );
extern void spawn( const Argument_t *argument );
extern void start_jdwmrc( void );
extern void update_borders( void );

// TODO: Order these somehow
extern Display *g_display;
extern Cursor *g_cursor[ CURSOR_COUNT ];
extern Window g_root_window;
extern unsigned int g_logging_level;
extern int g_unmanaged;
extern int g_screen;
extern int g_screen_width, g_screen_height;
extern int g_left_right_text_padding;
extern int g_theme_index;
extern bool g_restart, g_running;
extern char *g_user_home;

#endif /* CORE_H_ */

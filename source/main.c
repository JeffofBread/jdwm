#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include "cli.h"
#include "core.h"
#include "draw.h"
#include "logger.h"
#include "parser.h"
#include "util.h"

// TODO: Reduce the amount of logic present in @ref main() and generally clean up logical order
int main( const int argc, char *argv[ ] ) {

        TIMER_CREATE( main_timer );
        TIMER_INITIALIZE( main_timer );

        TIMER_CREATE( main_total_timer );
        TIMER_INITIALIZE( main_total_timer );

        // Make stdout/stderr line buffered. Copied from AwesomeWM
        setvbuf( stdout, NULL, _IOLBF, 0 );
        setvbuf( stderr, NULL, _IOLBF, 0 );

        jdwm_parse_args( argc, argv );

        g_user_home = getenv( "HOME" );

        // Initialize default logger(s)
        if ( !g_cli_flags[ FLAG_DISABLE_DEFAULT_LOGGER ] ) {
                logger_add_stream_handler( DEFAULT_LOGGER_NAME, DEFAULT_LOG_STREAM, g_logging_level, DEFAULT_LOG_STREAM_IS_QUIET,DEFAULT_LOG_STREAM_USE_COLOR );
        }

        // Print debug info related to CLI values
        if ( g_logging_level >= LOG_DEBUG ) {
                char *current_working_directory = getcwd( NULL, 0 );
                log_debug( "Program's working directory: \"%s\"", current_working_directory );
                free( current_working_directory );

                log_debug( "Option flags: " );
                log_debug( "\t\tFLAG_CUSTOM_CONFIG: %d", g_cli_flags[ FLAG_CUSTOM_CONFIG ] );
                log_debug( "\t\tFLAG_CUSTOM_LOG_LEVEL: %d", g_cli_flags[ FLAG_CUSTOM_LOG_LEVEL ] );
                log_debug( "\t\tFLAG_DISABLE_DEFAULT_LOGGER: %d", g_cli_flags[ FLAG_DISABLE_DEFAULT_LOGGER ] );
                log_debug( "\t\tFLAG_NO_JDWMRC: %d", g_cli_flags[ FLAG_NO_JDWMRC ] );
                log_debug( "\t\tCustom config path = \"%s\"", g_master_config.config_filepath );
                log_debug( "\t\tCurrent logging level = \"%d\"", g_logging_level );
        }

        log_info( "Initializing jdwm" );

        if ( !setlocale( LC_CTYPE, "" ) || !XSupportsLocale() ) {
                log_warn( "No locale support" );
        }

        if ( !( ( g_display = XOpenDisplay( NULL ) ) ) ) {
                log_fatal( "Cannot open display" );
        }

        check_for_other_window_manager();

        g_screen = DefaultScreen( g_display );
        g_screen_width = DisplayWidth( g_display, g_screen );
        g_screen_height = DisplayHeight( g_display, g_screen );
        g_root_window = RootWindow( g_display, g_screen );
        g_drawable = drawable_create( g_display, g_screen, g_root_window, g_screen_width, g_screen_height );

        TIMER_CHECKPOINT( main_timer, "Parsing CLI + early initialization" );

        // libconfig config parsing
        parse_config( &g_master_config );

        TIMER_CHECKPOINT( main_timer, "Parsing config" );

        setup();

        TIMER_CHECKPOINT( main_timer, "Setup" );

        #ifdef __OpenBSD__
        if ( pledge( "stdio rpath proc exec", NULL ) == -1 ) log_fatal( "pledge" );
        #endif /* __OpenBSD__ */

        scan();

        TIMER_CHECKPOINT( main_timer, "Scan" );

        TIMER_CHECKPOINT( main_total_timer, "Main" );

        run();

        if ( g_restart ) execvp( argv[ 0 ], build_restart_args() );

        cleanup();

        XCloseDisplay( g_display );

        return EXIT_SUCCESS;
}

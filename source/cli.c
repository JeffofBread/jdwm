#include "cli.h"

#include <parser.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "core.h"
#include "gnugetopt.h"
#include "logger.h"
#include "util.h"

bool g_cli_flags[ FLAGS_COUNT ];

static char *handle_config( const char *argument );
static char *handle_log_level( const char *argument );
static char *handle_version( const char *argument );
static char *handle_version_long( const char *argument );
static char *handle_usage( const char *argument );
static void print_usage( void );

// TODO: Look at adjusting these names + vals
static struct Cli_Option {
        struct option getopt_option;                 // the getopt_long struct
        const enum Jdwm_Flags flag_enum;             // Enum correlating to the index of cli_flags on master config
        const char *argument_name;                   // name of the argument if required, e.g. "FILE"
        const char *description;                     // description for usage/help
        char * ( *handler )( const char *argument ); // function handler to handle option
} CLI_OPTIONS[ ] = {
        { { "config", required_argument, NULL, 'C' }, FLAG_CUSTOM_CONFIG, "FILE", "Path to a custom config file", handle_config },
        { { "disable-default-logger", no_argument, NULL, 'D' }, FLAG_DISABLE_DEFAULT_LOGGER, NULL, "Disable default stdout logger", NULL },
        { { "log-level", required_argument, NULL, 'L' }, FLAG_CUSTOM_LOG_LEVEL, "LEVEL", "Set logging level (0-5)", handle_log_level },
        { { "no-jdwmrc", no_argument, NULL, 'N' }, FLAG_NO_JDWMRC, NULL, "Skip starting `~/.jdwmrc`", NULL },
        { { "version", no_argument, NULL, 'V' }, -1, NULL, "Show detailed version", handle_version_long },
        { { "simple-version", no_argument, NULL, 'v' }, -1, NULL, "Show simple version (no newline)", handle_version },
        { { "help", no_argument, NULL, 'h' }, -1, NULL, "Show help", handle_usage },
};

char **build_restart_args( void ) {

        int argc = 0, index = 0;

        // Always enable FLAG_NO_JDWMRC
        g_cli_flags[ FLAG_NO_JDWMRC ] = true;

        for ( int i = 0; i < FLAGS_COUNT; i++ ) {
                if ( g_cli_flags[ i ] ) {
                        argc++;
                        argc += CLI_OPTIONS[ i ].getopt_option.has_arg;
                }
        }

        // +2 for program name and null terminator
        char **args = ecalloc( argc + 2, sizeof( char * ) );
        args[ index++ ] = strdup( "jdwm" );

        // Create the beginnings of a short option like "-X\0"
        char tmp[ 3 ];
        tmp[ 0 ] = '-';
        tmp[ 2 ] = '\0';

        // Loop over array of flags and find matches between
        // enabled flags and their CLI_OPTIONS entry
        for ( int i = 0; i < FLAGS_COUNT; i++ ) {
                if ( !g_cli_flags[ i ] ) continue;
                for ( int j = 0; j < LENGTH_INT( CLI_OPTIONS ); j++ ) {
                        if ( (int) CLI_OPTIONS[ j ].flag_enum == i ) {

                                // Copy their val to tmp, making "-X\0"
                                tmp[ 1 ] = (char) CLI_OPTIONS[ j ].getopt_option.val;
                                args[ index++ ] = strdup( tmp );

                                // If it requires an argument, get it from the handler
                                if ( CLI_OPTIONS[ j ].getopt_option.has_arg ) {
                                        args[ index++ ] = strdup( CLI_OPTIONS[ j ].handler( NULL ) );
                                }

                                break;
                        }
                }
        }

        args[ index ] = NULL; // null terminate
        return args;
}

static char *handle_config( const char *argument ) {
        if ( argument == NULL ) return g_master_config.config_filepath;
        g_master_config.config_filepath = strdup( argument );
        return NULL;
}

static char *handle_log_level( const char *argument ) {

        if ( argument == NULL ) {
                char *level_str = malloc( 2 );
                snprintf( level_str, 2, "%d", g_logging_level );
                return level_str;
        }

        int tmp = (int) strtol( argument, NULL, 10 );
        if ( tmp < LOG_TRACE ) {
                fprintf( DEFAULT_LOG_STREAM, "[CLI Parser] handle_log_level(): given value (%d) less than LOG_TRACE (%d), clamping range.\n", tmp, LOG_TRACE );
                tmp = LOG_TRACE;
        } else if ( tmp > LOG_FATAL ) {
                fprintf( DEFAULT_LOG_STREAM, "[CLI Parser] handle_log_level(): given value (%d) greater than LOG_FATAL (%d), clamping range.\n", tmp, LOG_FATAL );
                tmp = LOG_FATAL;
        }

        g_logging_level = tmp;

        return NULL;
}

static char *handle_version( const char *argument ) {
        printf( VERSION );
        exit( EXIT_SUCCESS );
}

static char *handle_version_long( const char *argument ) {
        printf( "jdwm-" VERSION "\n" );
        exit( EXIT_SUCCESS );
}

static char *handle_usage( const char *argument ) {
        print_usage();
        exit( EXIT_SUCCESS );
}

// TODO: Look to accept only long options as well, avoid needing a short option
void jdwm_parse_args( const int argc, char **argv ) {

        // x2 just accounts for the possibility of extra ':'
        // characters for denoting a required argument.
        // It's longer than it needs to be, but it avoids
        // heap allocation.
        int short_options_index = 0;
        char short_options[ LENGTH( CLI_OPTIONS ) * 2 ] = { 0 };

        struct option long_options[ LENGTH( CLI_OPTIONS ) ] = { 0 };

        for ( int i = 0; i < LENGTH_INT( CLI_OPTIONS ); i++ ) {
                long_options[ i ] = CLI_OPTIONS[ i ].getopt_option;
                if ( CLI_OPTIONS[ i ].getopt_option.val ) {
                        short_options[ short_options_index++ ] = (char) CLI_OPTIONS[ i ].getopt_option.val;
                        if ( CLI_OPTIONS[ i ].getopt_option.has_arg ) {
                                short_options[ short_options_index++ ] = ':';
                        }
                }
        }

        short_options[ short_options_index ] = '\0';

        // Just makes code a little clearer
        #define selected_option_argument optarg

        int selected_option, options_index = 0;
        while ( ( selected_option = getopt_long( argc, argv, short_options, long_options, &options_index ) ) != -1 ) {
                bool found_value = false;
                for ( int i = 0; i < LENGTH_INT( CLI_OPTIONS ); i++ ) {
                        if ( CLI_OPTIONS[ i ].getopt_option.val == selected_option ) {
                                if ( (int) CLI_OPTIONS[ i ].flag_enum != -1 ) g_cli_flags[ CLI_OPTIONS[ i ].flag_enum ] = true;
                                if ( CLI_OPTIONS[ i ].handler ) CLI_OPTIONS[ i ].handler( selected_option_argument );
                                found_value = true;
                                break;
                        }
                }

                // If nothing matches an option in CLI_OPTIONS[]
                if ( !found_value ) {
                        print_usage();
                        exit( EXIT_FAILURE );
                }
        }
}

static void print_usage( void ) {
        printf( "Usage: jdwm [OPTIONS]\nOptions:\n" );

        int longest_option_length = 0;
        for ( int i = 0; i < LENGTH_INT( CLI_OPTIONS ); i++ ) {
                int tmp_length = (int) strlen( CLI_OPTIONS[ i ].getopt_option.name ) + 2;     // +2 for '--'
                if ( CLI_OPTIONS[ i ].getopt_option.val ) tmp_length += 4;                    // +4 for '-x, '
                if ( tmp_length > longest_option_length ) longest_option_length = tmp_length; // If we find a longer line, save length
        }

        for ( int i = 0; i < LENGTH_INT( CLI_OPTIONS ); i++ ) {
                putchar( '\t' );
                if ( CLI_OPTIONS[ i ].getopt_option.val ) printf( "-%c, ", CLI_OPTIONS[ i ].getopt_option.val );
                else printf( "    " );
                printf( "--%-*s %s\n", longest_option_length, CLI_OPTIONS[ i ].getopt_option.name, CLI_OPTIONS[ i ].description );
        }
}
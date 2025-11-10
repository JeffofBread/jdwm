// TODO: Add locking.
// TODO: Warn user if they try and use debug/trace in a build where it isn't available
// TODO: Allow user to select filemode of log from CLI.
// TODO: Trace at the entry/exit of every function with -finstrument-functions. https://gcc.gnu.org/onlinedocs/gcc-6.3.0/gcc/Instrumentation-Options.html

#include "logger.h"

#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "util.h"

static Logger_t s_logger = { .count = 0 };

const char *LOG_LEVEL_STRINGS[ ] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };
const char *LOG_LEVEL_COLORS[ ] = { "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m" };

// Static Function Definitions
static void dump_log( Log_Event_t *log_event, const Output_Handler_t *output_handler );
static int logger_add_handler( const char *name, void *file_pointer, unsigned int level, bool quiet, bool color );

static void dump_log( Log_Event_t *log_event, const Output_Handler_t *output_handler ) {

        // Store formatted time in buffer
        char time[ 32 ];
        time[ strftime( time, sizeof( time ), DATE_TIME_FORMAT, log_event->time ) ] = '\0';

        // Print logging info like line, file, log level, etc

        // @formatter:off
        if ( output_handler->color ) {
                fprintf(
                        (FILE *) output_handler->file_pointer,
                        COLOR_LOG_FORMAT,
                        time,
                        LOG_LEVEL_COLORS[ log_event->log_level ],
                        LOG_LEVEL_STRINGS[ log_event->log_level ],
                        log_event->filename,
                        log_event->function_name,
                        log_event->line
                );
        } else {
                fprintf(
                        (FILE *) output_handler->file_pointer,
                        NO_COLOR_LOG_FORMAT,
                        time,
                        LOG_LEVEL_STRINGS[ log_event->log_level ],
                        log_event->filename,
                        log_event->function_name,
                        log_event->line
                );
        }
        // @formatter:on

        // Print error
        vfprintf( output_handler->file_pointer, log_event->format, log_event->arguments );

        // Clean up end of print
        fprintf( output_handler->file_pointer, "\n" );
        fflush( output_handler->file_pointer );
}

static int logger_add_handler( const char *name, void *file_pointer, const unsigned int level, const bool quiet, const bool color ) {

        if ( s_logger.count == MAX_OUTPUT_HANDLERS ) {
                fprintf( stderr, "[Logger] logger_add_handler(): Maximum number of handlers reached: %d\n", MAX_OUTPUT_HANDLERS );
                return -1;
        }

        for ( unsigned int i = 0; i < s_logger.count; ++i ) {
                if ( strcmp( s_logger.output_handlers[ i ].name, name ) == 0 ) {
                        log_warn( "Logger %d already possesses the name \"%s\", cannot create another with the same name", i + 1, name );
                }
        }

        // @formatter:off
        s_logger.output_handlers[ s_logger.count++ ] = ( Output_Handler_t ){
                .name = name,
                .file_pointer = file_pointer,
                .level = level,
                .quiet = quiet,
                .color = color,
        };
        // @formatter:on

        log_debug( "Logger output handle created. name: \"%s\", file_pointer: \"%p\", level: %d, quiet: %d, color: %d", name, file_pointer, level, quiet, color );

        return 0;
}

int logger_add_file_handler( const char *handler_name, const char *filepath, const char *filemode, unsigned int log_level, const bool is_quiet, const bool use_color ) {

        if ( log_level > LOG_FATAL ) {
                fprintf( DEFAULT_LOG_STREAM, "[Logger] logger_add_file_handler(): log_level greater than LOG_FATAL (%d), value clamped.\n", LOG_FATAL );
                log_level = LOG_FATAL;
        }

        if ( handler_name == NULL || handler_name[ 0 ] == '\0' ) {
                fprintf( DEFAULT_LOG_STREAM, "[Logger] logger_add_file_handler(): handler_name was null or empty, cant create file logger.\n" );
                return -1;
        }

        if ( filepath == NULL || filepath[ 0 ] == '\0' ) {
                fprintf( DEFAULT_LOG_STREAM, "[Logger] logger_add_file_handler(): filepath was null or empty, cant create file logger.\n" );
                return -1;
        }

        if ( filemode == NULL || filemode[ 0 ] == '\0' ) {
                fprintf( DEFAULT_LOG_STREAM, "[Logger] logger_add_file_handler(): filemode was null or empty, cant create file logger.\n" );
                return -1;
        }

        char *expanded_filepath;
        expand_home_path( filepath, &expanded_filepath );

        char expanded_filepath_parent_directory[ PATH_MAX ];
        strncpy( expanded_filepath_parent_directory, expanded_filepath, PATH_MAX - 1 );
        expanded_filepath_parent_directory[ PATH_MAX - 1 ] = '\0';
        make_parent_directory( dirname( expanded_filepath_parent_directory ) );

        FILE *file_pointer = fopen( expanded_filepath, filemode );
        free( expanded_filepath );

        if ( !file_pointer ) {
                fprintf( DEFAULT_LOG_STREAM, "[Logger] logger_add_file_handler(): Unable to open file \"%s\"\n", filepath );
                return -1;
        }

        return logger_add_handler( handler_name, file_pointer, log_level, is_quiet, use_color );
}

int logger_add_stream_handler( const char *handler_name, FILE *file_pointer, unsigned int log_level, const bool is_quiet, const bool use_color ) {

        if ( log_level > LOG_FATAL ) {
                fprintf( DEFAULT_LOG_STREAM, "[Logger] logger_add_stream_handler(): log_level greater than LOG_FATAL (%d), value clamped.\n", LOG_FATAL );
                log_level = LOG_FATAL;
        }

        if ( handler_name == NULL || handler_name[ 0 ] == '\0' ) {
                fprintf( DEFAULT_LOG_STREAM, "[Logger] logger_add_stream_handler(): handler_name was null or empty, cant create file logger.\n" );
                return -1;
        }

        if ( file_pointer == NULL ) {
                fprintf( DEFAULT_LOG_STREAM, "[Logger] logger_add_stream_handler(): file_pointer was null, falling back to using default stream.\n" );
        }

        return logger_add_handler( handler_name, file_pointer, log_level, is_quiet, use_color );
}

int logger_set_log_level( const char *handler_name, const unsigned int log_level ) {

        if ( handler_name == NULL || handler_name[ 0 ] == '\0' ) {
                log_warn( "Unable to set log level of handler, given name was null: %s", handler_name );
                return -1;
        }

        if ( log_level > LOG_FATAL ) {
                log_warn( "Unable to set log level of handler \"%s\", log level was out of range", handler_name );
                return -1;
        }

        for ( unsigned int i = 0; i < s_logger.count; ++i ) {
                if ( strcmp( s_logger.output_handlers[ i ].name, handler_name ) == 0 ) {
                        s_logger.output_handlers[ i ].level = log_level;
                        log_debug( "Log level for handler \"%s\" successfully set to %s", handler_name, LOG_LEVEL_STRINGS[ log_level ] );
                        return 0;
                }
        }

        log_warn( "Unable to set log level of handler \"%s\", name did not match an existing handler", handler_name );

        return -1;
}

int logger_set_quiet( const char *handler_name, const bool quiet ) {

        if ( handler_name == NULL || handler_name[ 0 ] == '\0' ) {
                log_warn( "Unable to set quiet value of handler, given name was null" );
                return -1;
        }

        for ( unsigned int i = 0; i < s_logger.count; ++i ) {
                if ( strcmp( s_logger.output_handlers[ i ].name, handler_name ) == 0 ) {
                        s_logger.output_handlers[ i ].quiet = quiet;
                        log_debug( "Quiet value for handler \"%s\" successfully set to %d", handler_name, quiet );
                        return 0;
                }
        }

        log_warn( "Unable to set quiet value of handler \"%s\", name did not match an existing handler" );

        return -1;
}

void log_message( const unsigned int log_level, const char *filename, const char *function_name, const int line, const char *format, ... ) {

        // `filename + 3` is just to trim off the leading "../" in the string from __FILE__. When meson compiles the code,
        // and therefore the preprocessor is run and fills __FILE__, it is doing so from `./build/` relative to the root directory.
        Log_Event_t log_event = { .log_level = log_level, .filename = filename + 3, .function_name = function_name, .line = line, .format = format };

        const time_t temp_time = time( NULL );
        log_event.time = localtime( &temp_time );

        const Output_Handler_t *tmp_output_handler = NULL;
        for ( unsigned int i = 0; i < s_logger.count; ++i ) {
                tmp_output_handler = &s_logger.output_handlers[ i ];
                if ( tmp_output_handler && !tmp_output_handler->quiet && log_event.log_level >= tmp_output_handler->level ) {
                        va_start( log_event.arguments, format );
                        dump_log( &log_event, tmp_output_handler );
                        va_end( log_event.arguments );
                }
        }
}
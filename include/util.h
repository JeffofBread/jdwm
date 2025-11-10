#ifndef UTIL_H_
#define UTIL_H_

// TODO: Maybe come back to this file and look at using some features from picom's compiler.h

#include <stdio.h>
#include "logger.h"

#define SAFE_FREE( p ) do { if ( p ) { free( ( void * ) ( p ) ); ( p ) = NULL; } } while ( 0 )
#define SAFE_FCLOSE( f ) do { if ( f ) { fclose( f ); ( f ) = NULL; } } while ( 0 )

#define DEFAULT_CONFIG_DIRECTORY "/.config"
#define DEFAULT_DATA_DIRECTORY "/.local/share"

extern void *ecalloc( size_t nmemb, size_t size );
extern int expand_home_path( const char *input_path, char **expanded_path );
extern char *get_xdg_config_home( void );
extern char *get_xdg_data_home( void );
extern int make_parent_directory( const char *path );
extern char *malloc_string_join( const char *string_1, const char *string_2 );
extern void malloc_string_extend( char **source, const char *addition );
extern int normalize_path( const char *path, char **normalized_path );

static inline int clamp_range_int( const int input, const int min, const int max ) {
        if ( input < min ) {
                log_warn( "Clamped \"%d\" to a min of \"%d\"", input, min );
                return min;
        }

        if ( input > max ) {
                log_warn( "Clamped \"%d\" to a max of \"%d\"", input, max );
                return max;
        }

        return input;
}

static inline unsigned int clamp_range_uint( const unsigned int input, const unsigned int min, const unsigned int max ) {
        if ( input < min ) {
                log_warn( "Clamped \"%d\" to a min of \"%d\"", input, min );
                return min;
        }

        if ( input > max ) {
                log_warn( "Clamped \"%d\" to a max of \"%d\"", input, max );
                return max;
        }

        return input;
}

static inline long clamp_range_long( const long input, const long min, const long max ) {
        if ( input < min ) {
                log_warn( "Clamped \"%ld\" to a min of \"%ld\"", input, min );
                return min;
        }

        if ( input > max ) {
                log_warn( "Clamped \"%ld\" to a max of \"%ld\"", input, max );
                return max;
        }

        return input;
}

static inline unsigned long clamp_range_ulong( const unsigned long input, const unsigned long min, const unsigned long max ) {
        if ( input < min ) {
                log_warn( "Clamped \"%ld\" to a min of \"%ld\"", input, min );
                return min;
        }

        if ( input > max ) {
                log_warn( "Clamped \"%ld\" to a max of \"%ld\"", input, max );
                return max;
        }
        return input;
}

static inline float clamp_range_float( const float input, const float min, const float max ) {
        if ( input < min ) {
                log_warn( "Clamped \"%f\" to a min of \"%f\"", (double) input, (double) min );
                return min;
        }

        if ( input > max ) {
                log_warn( "Clamped \"%f\" to a max of \"%f\"", (double) input, (double) max );
                return max;
        }

        return input;
}

#ifdef _DEBUG

#include <time.h>
extern struct timespec g_timer;

static inline void timer_checkpoint( struct timespec *timer, const char *label ) {
        struct timespec now;
        clock_gettime( CLOCK_MONOTONIC, &now );
        const double elapsed_ms = (double) ( now.tv_sec - timer->tv_sec ) * 1000.0 + (double) ( now.tv_nsec - timer->tv_nsec ) / 1000000.0;
        log_info( "%s took ~%.3f ms", label, elapsed_ms );
        *timer = now;
}

#define TIMER_CREATE( name ) struct timespec name
#define TIMER_INITIALIZE( timer ) clock_gettime( CLOCK_MONOTONIC, &timer )
#define TIMER_CHECKPOINT( timer, label ) timer_checkpoint( &timer, label )

#else

#define TIMER_CREATE( name )
#define TIMER_INITIALIZE( timer )
#define TIMER_CHECKPOINT( timer, label )

#endif

#endif /* UTIL_H_ */

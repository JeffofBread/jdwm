#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "core.h"
#include "logger.h"

#ifdef _DEBUG
#include <time.h>
struct timespec g_timer;
#endif

void *ecalloc( const size_t nmemb, const size_t size ) {
        void *pointer;

        errno = 0;
        if ( !( ( pointer = calloc( nmemb, size ) ) ) ) {
                log_fatal( "ecalloc failed: nmemb = %d, size = %d: %s", nmemb, size, strerror( errno ) );
        }

        return pointer;
}

int expand_home_path( const char *input_path, char **expanded_path ) {
        if ( !g_user_home ) {
                log_warn( "Unable to get user's home directory" );
                return -1;
        }

        // Check if path starts with '~' or "$HOME"
        const size_t path_length = strlen( input_path );
        const size_t home_length = strlen( g_user_home );
        if ( input_path[ 0 ] == '~' && ( input_path[ 1 ] == '/' || input_path[ 1 ] == '\0' ) ) {
                *expanded_path = ecalloc( home_length + path_length, sizeof( char ) );
                strcpy( *expanded_path, g_user_home );
                strcat( *expanded_path, input_path + 1 );
        } else if ( strncmp( input_path, "$HOME", 5 ) == 0 && ( input_path[ 5 ] == '/' || input_path[ 5 ] == '\0' ) ) {
                *expanded_path = ecalloc( home_length + path_length - 5 + 1, sizeof( char ) );
                strcpy( *expanded_path, g_user_home );
                strcat( *expanded_path, input_path + 5 );
        } else {
                *expanded_path = strdup( input_path );
        }

        return 0;
}

char *get_xdg_config_home( void ) {
        char *xdg_config_home = getenv( "XDG_CONFIG_HOME" );

        if ( !xdg_config_home ) {
                if ( !g_user_home ) {
                        log_warn( "Unable to get user's home directory" );
                        return NULL;
                }

                xdg_config_home = malloc_string_join( g_user_home, DEFAULT_CONFIG_DIRECTORY );
        } else {
                xdg_config_home = strdup( xdg_config_home );
        }

        return xdg_config_home;
}

char *get_xdg_data_home( void ) {
        char *xdg_data_home = getenv( "XDG_DATA_HOME" );

        if ( !xdg_data_home ) {
                if ( !g_user_home ) {
                        log_warn( "Unable to get user's home directory" );
                        return NULL;
                }

                xdg_data_home = malloc_string_join( g_user_home, DEFAULT_DATA_DIRECTORY );
        } else {
                xdg_data_home = strdup( xdg_data_home );
        }

        return xdg_data_home;
}

int make_parent_directory( const char *path ) {
        char *normalized_path;
        normalize_path( path, &normalized_path );
        const char *walk = normalized_path;

        const size_t normalized_path_length = strlen( normalized_path );

        while ( walk < normalized_path + normalized_path_length + 1 ) {

                // Get length from walk to next '/'
                const size_t n = strcspn( walk, "/" );

                // Skip path '/'
                if ( n == 0 ) {
                        walk++;
                        continue;
                }

                // Length of current path segment
                const size_t current_path_length = walk - normalized_path + n;
                char current_path[ current_path_length + 1 ];

                // Copy path segment to stat
                strncpy( current_path, normalized_path, current_path_length );
                strcpy( current_path + current_path_length, "" );

                struct stat tmp_stat;
                const int stat_return = stat( current_path, &tmp_stat );

                if ( stat_return < 0 ) {
                        if ( errno == ENOENT ) {
                                log_debug( "Making directory %s", current_path );
                                if ( mkdir( current_path, 0700 ) < 0 ) {
                                        log_error( "Failed to make directory %s", current_path );
                                        perror( "" );
                                        free( normalized_path );
                                        return -1;
                                }
                        } else {
                                log_error( "Error stat-ing directory %s", current_path );
                                perror( "" );
                                free( normalized_path );
                                return -1;
                        }
                }

                // Continue to next path segment
                walk += n;
        }

        free( normalized_path );

        return 0;
}

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

// gcc warns about legitimate truncation worries in strncpy in malloc_string_join.
// strncpy( joined_string, string_1, length_1 ) intentionally truncates the null byte
// from string_1, however. strncpy( joined_string + length_1, string_2, length_2 )
// uses bounds depending on the source argument, but joined_string is allocated with
// length_1 + length_2 + 1, so this strncpy can't overflow.
//
// Allocate the space and join two strings. - Derived from picom ( str.c::mstrjoin() )
char *malloc_string_join( const char *string_1, const char *string_2 ) {
        const size_t length_1 = strlen( string_1 );
        const size_t length_2 = strlen( string_2 );
        const size_t total_length = length_1 + length_2 + 1;

        char *joined_string = ecalloc( total_length, sizeof( char ) );

        strncpy( joined_string, string_1, length_1 );
        strncpy( joined_string + length_1, string_2, length_2 );
        joined_string[ total_length - 1 ] = '\0';

        return joined_string;
}

// Concatenate a string on heap with another string. - Derived from picom ( str.c::mstrextend() )
void malloc_string_extend( char **source, const char *addition ) {
        if ( !*source ) {
                *source = strdup( addition );
                return;
        }

        const size_t source_length = strlen( *source );
        const size_t addition_length = strlen( addition );
        const size_t new_length = source_length + addition_length + 1;

        *source = realloc( *source, new_length );

        strncpy( *source + source_length, addition, addition_length );
        ( *source )[ new_length - 1 ] = '\0';
}

#ifndef __clang__
#pragma GCC diagnostic pop
#endif

int normalize_path( const char *path, char **normalized_path ) {
        const size_t path_length = strlen( path );
        *normalized_path = (char *) malloc( ( path_length + 1 ) * sizeof( char ) );

        size_t new_length = 0;
        const char *match, *walk = path;
        while ( ( match = strchr( walk, '/' ) ) ) {

                // Copy everything between match and walk
                strncpy( *normalized_path + new_length, walk, match - walk );
                new_length += match - walk;
                walk += match - walk;

                // Skip all repeating slashes
                while ( *walk == '/' ) {
                        walk++;
                }

                // If not last character in path
                if ( walk != path + path_length ) {
                        ( *normalized_path )[ new_length++ ] = '/';
                }
        }

        ( *normalized_path )[ new_length++ ] = '\0';

        // Copy remaining path
        strcat( *normalized_path, walk );
        new_length += strlen( walk );

        *normalized_path = (char *) realloc( *normalized_path, new_length * sizeof( char ) );

        return 0;
}

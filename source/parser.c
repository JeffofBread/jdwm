#include "parser.h"

#include <ctype.h>
#include <errno.h>
#include <libconfig.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xft/Xft.h>
#include "bar.h"
#include "cli.h"
#include "client.h"
#include "common.h"
#include "core.h"
#include "draw.h"
#include "gaps.h"
#include "layout.h"
#include "logger.h"
#include "monitor.h"
#include "tags.h"
#include "util.h"

Master_Config_t g_master_config = { 0 };

// Private / Static internal functions
static void backup_config( config_t *config );
static void load_default_bar_config( Bar_Config_t *bar_config );
static void load_default_buttonbind_config( Button_t **buttonbind_config, unsigned int *buttonbind_count, bool *default_buttonbinds_loaded );
static void load_default_keybind_config( Key_t **keybind_config, unsigned int *keybind_count, bool *default_keybinds_loaded );
static void load_default_layout_config( Layout_Config_t *layout_config );
static void load_default_master_config( Master_Config_t *master_config );
static void load_default_theme_config( Theme_Config_t **theme_config, unsigned int *themes_count );
static void load_default_window_config( Window_Config_t *window_config );
static int open_config( config_t *config, Master_Config_t *master_config );
static int parse_bind_argument( const char *argument_string, const enum Argument_Type *arg_type, Argument_t *arg, long double range_min, long double range_max );
static int parse_bind_function( const char *function_string, enum Argument_Type *arg_type, void ( **function )( const Argument_t * ), long double *range_min, long double *range_max );
static int parse_bind_modifier( const char *modifier_string, unsigned int *modifier );
static int parse_buttonbind( const char *buttonbind_string, Button_t *buttonbind, unsigned int max_keys );
static int parse_buttonbind_button( const char *button_string, unsigned int *button );
static int parse_buttonbind_click( const char *click_string, unsigned int *click );
static int parse_buttonbinds_config( const config_t *config, Button_t **buttonbind_config, unsigned int *buttonbind_count, unsigned int max_keys, bool *default_buttonbinds_loaded );
static int parse_generic_settings( const config_t *config, Master_Config_t *master_config );
static int parse_keybind( const char *keybind_string, Key_t *keybind, unsigned int max_keys );
static int parse_keybind_keysym( const char *keysym_string, KeySym *keysym );
static int parse_keybinds_config( const config_t *config, Key_t **keybind_config, unsigned int *keybinds_count, unsigned int max_keys, bool *default_keybinds_loaded );
static int parse_logger_settings( const config_t *config );
static int parse_rules_string( const char *input_string, char *output_string, int output_string_size );
static int parse_rules_config( const config_t *config, Rule_t **rules_config, unsigned int *rules_count );
static int parse_tags_config( const config_t *config, Bar_Config_t *bar_config );
static int parse_theme( config_setting_t *theme, Theme_Config_t *theme_config, int theme_index );
static int parse_theme_client( config_setting_t *theme, const char *path, XftColor *client_theme, int theme_index );
static int parse_theme_bar_section( config_setting_t *theme, const char *path, XftColor *client_theme, int theme_index );
static int parse_themes_config( const config_t *config, Theme_Config_t **themes_config, unsigned int *themes_count );

static inline int libconfig_lookup_bool( const config_t *config, const char *path, bool *value, const bool optional ) {
        int tmp = 0;
        if ( config_lookup_bool( config, path, &tmp ) != CONFIG_TRUE ) {
                if ( optional ) {
                        log_debug( "Optional value \"%s\" not found, skipping", path );
                        return 0;
                }
                log_warn( "Problem reading required config value \"%s\": Not found or of wrong type", path );
                return -1;
        }
        *value = tmp;
        return 0;
}

static inline int libconfig_setting_lookup_bool( const config_setting_t *setting, const char *path, bool *value, const bool optional ) {
        int tmp = 0;
        if ( config_setting_lookup_bool( setting, path, &tmp ) != CONFIG_TRUE ) {
                if ( optional ) {
                        log_debug( "Optional value \"%s\" not found, skipping", path );
                        return 0;
                }
                log_warn( "Problem reading required config value \"%s\": Not found or of wrong type", path );
                return -1;
        }
        *value = tmp;
        return 0;
}

static inline int libconfig_lookup_int( const config_t *config, const char *path, int *value, const bool optional, const int range_min, const int range_max ) {
        if ( config_lookup_int( config, path, value ) != CONFIG_TRUE ) {
                if ( optional ) {
                        log_debug( "Optional value \"%s\" not found, skipping", path );
                        return 0;
                }
                log_warn( "Problem reading required config value \"%s\": Not found or of wrong type", path );
                return -1;
        }
        *value = clamp_range_int( *value, range_min, range_max );
        return 0;
}

static inline int libconfig_setting_lookup_int( const config_setting_t *setting, const char *path, int *value, const bool optional, const int range_min, const int range_max ) {
        if ( config_setting_lookup_int( setting, path, value ) != CONFIG_TRUE ) {
                if ( optional ) {
                        log_debug( "Optional value \"%s\" not found, skipping", path );
                        return 0;
                }
                log_warn( "Problem reading required config value \"%s\": Not found or of wrong type", path );
                return -1;
        }
        *value = clamp_range_int( *value, range_min, range_max );
        return 0;
}

static inline int libconfig_lookup_uint( const config_t *config, const char *path, unsigned int *value, const bool optional, const unsigned int range_min, const unsigned int range_max ) {
        int tmp = 0;
        if ( config_lookup_int( config, path, &tmp ) != CONFIG_TRUE ) {
                if ( optional ) {
                        log_debug( "Optional value \"%s\" not found, skipping", path );
                        return 0;
                }
                log_warn( "Problem reading required config value \"%s\": Not found or of wrong type", path );
                return -1;
        }
        *value = clamp_range_uint( tmp, range_min, range_max );
        return 0;
}

static inline int libconfig_setting_lookup_uint( const config_setting_t *setting, const char *path, unsigned int *value, const bool optional, const unsigned int range_min,
                                                 const unsigned int range_max ) {
        int tmp = 0;
        if ( config_setting_lookup_int( setting, path, &tmp ) != CONFIG_TRUE ) {
                if ( optional ) {
                        log_debug( "Optional value \"%s\" not found, skipping", path );
                        return 0;
                }
                log_warn( "Problem reading required config value \"%s\": Not found or of wrong type", path );
                return -1;
        }
        *value = clamp_range_uint( tmp, range_min, range_max );
        return 0;
}

static inline int libconfig_lookup_string( const config_t *config, const char *path, const char **value, const bool optional ) {
        if ( config_lookup_string( config, path, value ) != CONFIG_TRUE ) {
                if ( optional ) {
                        log_debug( "Optional value \"%s\" not found, skipping", path );
                        return 0;
                }
                log_warn( "Problem reading required config value \"%s\": Not found or of wrong type", path );
                return -1;
        }
        return 0;
}

static inline int libconfig_setting_lookup_string( const config_setting_t *setting, const char *path, const char **value, const bool optional ) {
        if ( config_setting_lookup_string( setting, path, value ) == CONFIG_FALSE ) {
                if ( optional ) {
                        log_debug( "Optional value \"%s\" not found, skipping", path );
                        return 0;
                }
                log_warn( "Problem reading required config value \"%s\": Not found or of wrong type", path );
                return -1;
        }
        return 0;
}

static inline int libconfig_lookup_color( const config_t *config, const char *path, XftColor *value, const bool optional ) {
        const char *tmp = NULL;
        if ( libconfig_lookup_string( config, path, &tmp, optional ) ) return -1;
        return drawable_color_create( g_drawable, value, tmp );
}

static inline int libconfig_setting_lookup_color( const config_setting_t *setting, const char *path, XftColor *value, const bool optional ) {
        const char *tmp = NULL;
        if ( libconfig_setting_lookup_string( setting, path, &tmp, optional ) ) return -1;
        return drawable_color_create( g_drawable, value, tmp );
}

static inline char *trim_whitespace( char *string ) {
        if ( !string ) return NULL;
        while ( isspace( (unsigned char) *string ) ) string++;
        if ( *string == '\0' ) return string;
        char *end = string + strlen( string ) - 1;
        while ( end > string && isspace( (unsigned char) *end ) ) end--;
        *( end + 1 ) = '\0';
        return string;
}

// ----- Function Definitions -----

static void backup_config( config_t *config ) {

        if ( config == NULL ) {
                log_error( "Pointer to libconfig context is null" );
                return;
        }

        char *buffer = get_xdg_data_home();

        if ( buffer == NULL ) {
                log_error( "Unable to get necessary directory to backup config" );
                return;
        }

        // Append buffer (already has "~/.local/share" or other xdg data directory)
        // with the directory we want to backup the config to, create the directory
        // if it doesn't exist, and then append with the filename we want to backup
        // to config in.
        malloc_string_extend( &buffer, "/jdwm/" );
        if ( make_parent_directory( buffer ) ) {
                log_warn( "Unable to make parent directory \"%s\", backing up config may fail", buffer );
        }
        malloc_string_extend( &buffer, "jdwm_last.conf" );

        if ( config_write_file( config, buffer ) == CONFIG_FALSE ) {
                log_error( "Problem backing up current config to \"%s\"", buffer );
        } else {
                log_info( "Current config backed up to \"%s\"", buffer );
        }

        SAFE_FREE( buffer );
}

static void load_default_bar_config( Bar_Config_t *bar_config ) {

        if ( bar_config == NULL ) {
                log_fatal( "bar_config is null, can't load default configuration" );
        }

        // Bar Feature Toggles
        bar_config->show_by_default = true;
        bar_config->show_system_tray = true;
        bar_config->show_status_text = true;
        bar_config->show_tags = true;
        bar_config->show_client_title = true;
        bar_config->show_layout_symbol = true;
        bar_config->show_floating_indicator = true;
        bar_config->show_tag_underline = true;
        bar_config->enable_top_bar = true;
        bar_config->enable_centered_client_title = true;

        // Generic Paddings
        bar_config->vertical_bar_padding = 10;
        bar_config->horizontal_bar_padding = 10;
        bar_config->vertical_bar_text_padding = 1;
        bar_config->horizontal_bar_text_padding = 5;

        // Tag Underline Values
        bar_config->underline_thickness = 2;
        bar_config->underline_vertical_padding = 1;
        bar_config->underline_horizontal_padding = 5;

        // System Tray
        bar_config->system_tray_padding = 0;
        bar_config->system_tray_pinning = 0;

        // Client's Icon
        bar_config->client_icon_size = 20;
        bar_config->client_icon_padding = 5;

        // Misc
        bar_config->tags[ 0 ] = strdup( "1" );
        bar_config->tags[ 1 ] = strdup( "2" );
        bar_config->tags[ 2 ] = strdup( "3" );
        bar_config->tags[ 3 ] = strdup( "4" );
        bar_config->tags[ 4 ] = strdup( "5" );
        bar_config->tags[ 5 ] = strdup( "6" );
        bar_config->tags[ 6 ] = strdup( "7" );
        bar_config->tags[ 7 ] = strdup( "8" );
        bar_config->tags[ 8 ] = strdup( "9" );
}

static void load_default_buttonbind_config( Button_t **buttonbind_config, unsigned int *buttonbind_count, bool *default_buttonbinds_loaded ) {

        static Button_t default_buttonbinds[ ] = {

                // Core functionality
                { CLICK_WINDOW_TITLE, 0, Button1, &spawn, { .v = "xterm" }, ARG_TYPE_POINTER },
                { CLICK_STATUS_TEXT, 0, Button1, &spawn, { .v = "xterm" }, ARG_TYPE_POINTER },
                { CLICK_DESKTOP, 0, Button1, &spawn, { .v = "xterm" }, ARG_TYPE_POINTER },
                { CLICK_TAG, 0, Button1, &view_tag_bitmask, { .ui = 0 }, ARG_TYPE_UINT },
                { CLICK_WINDOW_TITLE, 0, Button2, &kill_client, { 0 }, ARG_TYPE_NONE },

                // Misc / Nice-to-haves
                { CLICK_CLIENT_WINDOW, Mod4Mask, Button1, &move_mouse, { 0 }, ARG_TYPE_NONE },
                { CLICK_CLIENT_WINDOW, Mod4Mask, Button3, &resize_mouse, { 0 }, ARG_TYPE_NONE },
                { CLICK_CLIENT_WINDOW, Mod4Mask | ControlMask, Button1, &float_and_move, { 0 }, ARG_TYPE_NONE },
                { CLICK_LAYOUT_SYMBOL, 0, Button1, &layout_scroll, { .i = 1 }, ARG_TYPE_INT },
                { CLICK_LAYOUT_SYMBOL, 0, Button3, &layout_scroll, { .i = -1 }, ARG_TYPE_INT },
                { CLICK_TAG, Mod4Mask, Button1, &client_to_tag_bitmask, { .ui = 0 }, ARG_TYPE_UINT },
                { CLICK_TAG, Mod4Mask, Button1, &view_tag_bitmask, { .ui = 0 }, ARG_TYPE_UINT },
        };

        *buttonbind_count = LENGTH( default_buttonbinds );
        *buttonbind_config = default_buttonbinds;
        *default_buttonbinds_loaded = true;
}

static void load_default_keybind_config( Key_t **keybind_config, unsigned int *keybind_count, bool *default_keybinds_loaded ) {

        // I may choose to add more keybinds to this in the future, but for now, I think this is more than enough
        static Key_t default_keybinds[ ] = {

                // Core functionality
                { Mod4Mask, XK_Return, &spawn, { .v = "xterm" }, ARG_TYPE_POINTER },
                { Mod4Mask, XK_q, &kill_client, { 0 }, ARG_TYPE_NONE },
                { Mod4Mask | XK_Shift_L, XK_q, &restart, { 0 }, ARG_TYPE_INT },
                { Mod4Mask | XK_Control_L | XK_Shift_L, XK_q, &quit, { 0 }, ARG_TYPE_INT },
                { Mod4Mask | XK_Shift_L, XK_space, &toggle_floating, { 0 }, ARG_TYPE_NONE },
                { Mod4Mask, XK_Up, &focus_stack, { .i = 1 }, ARG_TYPE_INT },
                { Mod4Mask, XK_Down, &focus_stack, { .i = -1 }, ARG_TYPE_INT },
                { Mod4Mask, XK_Left, &focus_monitor, { .i = -1 }, ARG_TYPE_INT },
                { Mod4Mask, XK_Right, &focus_monitor, { .i = 1 }, ARG_TYPE_INT },
                { Mod4Mask | XK_Shift_L, XK_Left, &focus_monitor, { .i = -1 }, ARG_TYPE_INT },
                { Mod4Mask | XK_Shift_L, XK_Right, &focus_monitor, { .i = 1 }, ARG_TYPE_INT },
                { Mod4Mask | XK_Shift_L, XK_Left, &client_to_monitor, { .i = -1 }, ARG_TYPE_INT },
                { Mod4Mask | XK_Shift_L, XK_Right, &client_to_monitor, { .i = 1 }, ARG_TYPE_INT },

                // View tags
                { Mod4Mask, XK_0, view_tag_bitmask, { .ui = ( ~0 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_1, view_tag_bitmask, { .ui = ( 1 << 0 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_2, view_tag_bitmask, { .ui = ( 1 << 1 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_3, view_tag_bitmask, { .ui = ( 1 << 2 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_4, view_tag_bitmask, { .ui = ( 1 << 3 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_5, view_tag_bitmask, { .ui = ( 1 << 4 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_6, view_tag_bitmask, { .ui = ( 1 << 5 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_7, view_tag_bitmask, { .ui = ( 1 << 6 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_8, view_tag_bitmask, { .ui = ( 1 << 7 ) }, ARG_TYPE_UINT },
                { Mod4Mask, XK_9, view_tag_bitmask, { .ui = ( 1 << 8 ) }, ARG_TYPE_UINT },
        };

        *keybind_count = LENGTH( default_keybinds );
        *keybind_config = default_keybinds;
        *default_keybinds_loaded = true;
}

static void load_default_layout_config( Layout_Config_t *layout_config ) {

        if ( layout_config == NULL ) {
                log_fatal( "layout_config is null, can't load default configuration" );
        }

        // Toggles
        layout_config->force_vertical_split = true;
        layout_config->respect_resize_hints = true;
        layout_config->smart_gaps = false;

        // Default Gaps
        layout_config->default_number_of_clients_in_master_area = 1;
        layout_config->default_master_factor = 50;
        layout_config->default_inner_horizontal_gap = 10;
        layout_config->default_inner_vertical_gap = 10;
        layout_config->default_outer_horizontal_gap = 10;
        layout_config->default_outer_vertical_gap = 10;
}

static void load_default_master_config( Master_Config_t *master_config ) {

        if ( master_config == NULL ) {
                log_fatal( "master_config is null, can't load default configuration" );
        }

        master_config->fallback_config_loaded = false;
        master_config->default_keybinds_loaded = false;
        master_config->default_buttonbinds_loaded = false;
        master_config->enable_system_tray_pin_to_first_monitor_on_fail = true;

        master_config->max_keys = 4;
        master_config->mouse_refresh_rate = 120;

        master_config->themes_count = 0;
        master_config->themes = NULL;

        master_config->rules_count = 0;
        master_config->rules = NULL;

        master_config->keybinds_count = 0;
        master_config->keybinds = NULL;

        master_config->buttonbinds_count = 0;
        master_config->buttonbinds = NULL;
}

// TODO: Deal with the returns of @ref drawable_color_create().
static void load_default_theme_config( Theme_Config_t **theme_config, unsigned int *themes_count ) {

        if ( theme_config == NULL ) {
                log_fatal( "theme_config is null, can't load default configuration" );
        }

        // Basic colors for the storm theme
        const char darkblue[ ] = "#16162F";
        const char lightblue[ ] = "#A8CAD3";
        const char orange[ ] = "#fa9061";

        *themes_count = 1;
        *theme_config = ecalloc( *themes_count, sizeof( Theme_Config_t ) );

        snprintf( ( *theme_config )->name, sizeof( ( *theme_config )->name ), "Default" );
        snprintf( ( *theme_config )->font, sizeof( ( *theme_config )->font ), "JetBrainsMono:size=14" );

        drawable_color_create( g_drawable, &( *theme_config )->color_systray, darkblue );

        drawable_color_create( g_drawable, &( *theme_config )->color_client_selected[ NORMAL_BORDER ], lightblue );
        drawable_color_create( g_drawable, &( *theme_config )->color_client_selected[ FLOATING_BORDER ], lightblue );

        drawable_color_create( g_drawable, &( *theme_config )->color_client_unselected[ NORMAL_BORDER ], darkblue );
        drawable_color_create( g_drawable, &( *theme_config )->color_client_unselected[ FLOATING_BORDER ], darkblue );

        drawable_color_create( g_drawable, &( *theme_config )->color_layout[ FOREGROUND ], lightblue );
        drawable_color_create( g_drawable, &( *theme_config )->color_layout[ BACKGROUND ], darkblue );

        drawable_color_create( g_drawable, &( *theme_config )->color_statusbar[ FOREGROUND ], lightblue );
        drawable_color_create( g_drawable, &( *theme_config )->color_statusbar[ BACKGROUND ], darkblue );

        drawable_color_create( g_drawable, &( *theme_config )->color_tag_selected[ FOREGROUND ], orange );
        drawable_color_create( g_drawable, &( *theme_config )->color_tag_selected[ BACKGROUND ], darkblue );

        drawable_color_create( g_drawable, &( *theme_config )->color_tag_unselected[ FOREGROUND ], lightblue );
        drawable_color_create( g_drawable, &( *theme_config )->color_tag_unselected[ BACKGROUND ], darkblue );

        drawable_color_create( g_drawable, &( *theme_config )->color_titlebar_selected[ FOREGROUND ], lightblue );
        drawable_color_create( g_drawable, &( *theme_config )->color_titlebar_selected[ BACKGROUND ], darkblue );

        drawable_color_create( g_drawable, &( *theme_config )->color_titlebar_unselected[ FOREGROUND ], lightblue );
        drawable_color_create( g_drawable, &( *theme_config )->color_titlebar_unselected[ BACKGROUND ], darkblue );
}

static void load_default_window_config( Window_Config_t *window_config ) {

        if ( window_config == NULL ) {
                log_fatal( "window_config is null, can't load default configuration" );
        }

        window_config->normal_border_width = 0;
        window_config->floating_border_width = 5;
        window_config->snapping_pixel_distance = 32;
        window_config->enable_raise_floating_windows_on_focus = true;

        snprintf( window_config->scratchpad_name, sizeof( window_config->scratchpad_name ), "Scratchpad" );
}

static int open_config( config_t *config, Master_Config_t *master_config ) {

        int i, config_filepaths_length = 0;
        char *config_filepaths[ 5 ];

        // Check if a custom user config was passed in and copy it if it was
        if ( master_config->config_filepath != NULL ) {
                config_filepaths[ config_filepaths_length++ ] = strdup( master_config->config_filepath );
        }

        // ~/.config/jdwm.conf
        char *config_top_directory = get_xdg_config_home();
        malloc_string_extend( &config_top_directory, "/jdwm.conf" );
        config_filepaths[ config_filepaths_length++ ] = config_top_directory;

        // ~/.config/jdwm/jdwm.conf
        char *config_sub_directory = get_xdg_config_home();
        malloc_string_extend( &config_sub_directory, "/jdwm/jdwm.conf" );
        config_filepaths[ config_filepaths_length++ ] = config_sub_directory;

        // ~/.local/share/jdwm/jdwm_last.conf
        char *config_backup = get_xdg_data_home();
        malloc_string_extend( &config_backup, "/jdwm/jdwm_last.conf" );
        config_filepaths[ config_filepaths_length++ ] = config_backup;

        // /etc/jdwm/jdwm.conf
        char *config_fallback = strdup( "/etc/jdwm/jdwm.conf" );
        config_filepaths[ config_filepaths_length++ ] = config_fallback;

        FILE *tmp_file = NULL;
        for ( i = 0; i < config_filepaths_length; i++ ) {
                log_debug( "Attempting to open config file \"%s\"", config_filepaths[ i ] );

                if ( config_filepaths[ i ] == NULL ) {
                        log_warn( "config_filepaths[%d] was null, unable to lookup intended config. Likely a memory allocation error", i );
                        continue;
                }

                tmp_file = fopen( config_filepaths[ i ], "r" );

                if ( tmp_file == NULL ) {
                        log_warn( "Unable to open config file \"%s\"", config_filepaths[ i ] );
                        continue;
                }

                if ( config_read( config, tmp_file ) == CONFIG_FALSE ) {
                        log_warn( "Problem parsing config file \"%s\", line %d: %s", config_filepaths[ i ], config_error_line( config ), config_error_text( config ) );
                        SAFE_FCLOSE( tmp_file );
                        continue;
                }

                // Save found config filepath
                SAFE_FREE( master_config->config_filepath );
                master_config->config_filepath = strdup( config_filepaths[ i ] );

                // Check if it's a user's custom configuration
                if ( strcmp( config_filepaths[ i ], config_backup ) == 0 || strcmp( config_filepaths[ i ], config_fallback ) == 0 ) {
                        master_config->fallback_config_loaded = true;
                }

                for ( i = 0; i < config_filepaths_length; i++ ) {
                        SAFE_FREE( config_filepaths[ i ] );
                }

                SAFE_FCLOSE( tmp_file );

                return 0;
        }

        log_error( "Unable to load any configs. Loading hardcoded default config values and exiting parsing" );
        load_default_keybind_config( &master_config->keybinds, &master_config->keybinds_count, &master_config->default_keybinds_loaded );
        load_default_buttonbind_config( &master_config->buttonbinds, &master_config->buttonbinds_count, &master_config->default_buttonbinds_loaded );

        for ( i = 0; i < config_filepaths_length; i++ ) {
                SAFE_FREE( config_filepaths[ i ] );
        }

        config_destroy( config );
        SAFE_FCLOSE( tmp_file );

        return -1;
}

void config_cleanup( Master_Config_t *master_config ) {
        int i;

        SAFE_FREE( master_config->config_filepath );

        SAFE_FREE( master_config->themes );

        for ( i = 0; i < LENGTH( master_config->bar_config.tags ); i++ ) {
                SAFE_FREE( master_config->bar_config.tags[ i ] );
        }

        SAFE_FREE( master_config->rules );

        if ( !master_config->default_keybinds_loaded ) {
                for ( i = 0; i < master_config->keybinds_count; i++ ) {
                        if ( master_config->keybinds[ i ].argument_type == ARG_TYPE_POINTER ) {
                                SAFE_FREE( master_config->keybinds[ i ].argument.v );
                        }
                }
                SAFE_FREE( master_config->keybinds );
        }

        if ( !master_config->default_buttonbinds_loaded ) {
                for ( i = 0; i < master_config->buttonbinds_count; i++ ) {
                        if ( master_config->buttonbinds[ i ].argument_type == ARG_TYPE_POINTER ) {
                                SAFE_FREE( master_config->buttonbinds[ i ].argument.v );
                        }
                }
                SAFE_FREE( master_config->buttonbinds );
        }
}

int parse_config( Master_Config_t *master_config ) {

        config_t libconfig_config;

        TIMER_CREATE( parsing_timer );
        TIMER_INITIALIZE( parsing_timer );

        // Initialize libconfig context
        config_init( &libconfig_config );

        // Populate master jdwm configuration with default values
        load_default_master_config( master_config );
        load_default_bar_config( &master_config->bar_config );
        load_default_layout_config( &master_config->layout_config );
        load_default_window_config( &master_config->window_config );
        load_default_theme_config( &master_config->themes, &master_config->themes_count );

        TIMER_CHECKPOINT( parsing_timer, "Loading default config" );

        if ( open_config( &libconfig_config, master_config ) ) return -1;

        TIMER_CHECKPOINT( parsing_timer, "Opening and reading config file" );

        log_info( "Path to config file: \"%s\"", master_config->config_filepath );

        // @formatter:off
        {
                char *config_include_directory = realpath( master_config->config_filepath, NULL );
                config_include_directory = dirname( config_include_directory );

                if ( config_include_directory ) {
                        config_set_include_dir( &libconfig_config, config_include_directory );
                } else {
                        log_error( "Unable to resolve configuration include directory" );
                }

                SAFE_FREE( config_include_directory );
        }
        // @formatter:on

        config_set_options( &libconfig_config, CONFIG_OPTION_AUTOCONVERT | CONFIG_OPTION_SEMICOLON_SEPARATORS );
        config_set_tab_width( &libconfig_config, 4 );

        TIMER_CHECKPOINT( parsing_timer, "Setting libconfig settings" );

        // Note: I may want to come back to this and think about how I handle these returns.
        // The return values from the functions aren't the greatest and I may want a threshold
        // or severity based on the error.
        int errors = 0;
        errors += parse_logger_settings( &libconfig_config );
        errors += parse_generic_settings( &libconfig_config, master_config );
        errors += parse_keybinds_config( &libconfig_config, &master_config->keybinds, &master_config->keybinds_count, master_config->max_keys, &master_config->default_keybinds_loaded );
        errors += parse_buttonbinds_config( &libconfig_config, &master_config->buttonbinds, &master_config->buttonbinds_count, master_config->max_keys, &master_config->default_buttonbinds_loaded );
        errors += parse_themes_config( &libconfig_config, &master_config->themes, &master_config->themes_count );
        errors += parse_rules_config( &libconfig_config, &master_config->rules, &master_config->rules_count );
        errors += parse_tags_config( &libconfig_config, &master_config->bar_config );

        TIMER_CHECKPOINT( parsing_timer, "Parsing all config items" );

        if ( master_config->fallback_config_loaded ) {
                log_warn( "Not saving config as backup, as current working config is not the user's" );
        } else if ( master_config->default_keybinds_loaded || master_config->default_buttonbinds_loaded ) {
                log_warn( "Not saving config as backup, as current working config uses default key or button binds" );
        } else if ( errors != 0 ) {
                log_warn( "Not saving config as backup, as current working config contains %d errors", errors );
        } else {
                backup_config( &libconfig_config );
        }

        TIMER_CHECKPOINT( parsing_timer, "Saving last good config" );

        config_destroy( &libconfig_config );

        TIMER_CHECKPOINT( parsing_timer, "Destroying Libconfig context" );

        return 0;
}

static int parse_bind_argument( const char *argument_string, const enum Argument_Type *arg_type, Argument_t *arg, const long double range_min, const long double range_max ) {

        log_trace( "Argument being parsed: \"%s\"", argument_string );

        if ( !argument_string || argument_string[ 0 ] == '\0' ) {
                log_error( "null or empty string passed to parse_bind_argument()" );
                return -1;
        }

        // @formatter:off
        char *end_pointer;
        switch ( *arg_type ) {
                case ARG_TYPE_INT:
                        arg->i = (int) clamp_range_long( strtol( argument_string, &end_pointer, 10 ), (long) range_min, (long) range_max );
                        if ( *end_pointer != '\0' ) return -1;
                        log_trace( "Argument type int: %ld", arg->i );
                        break;
                case ARG_TYPE_UINT:
                        arg->ui = (unsigned int) clamp_range_long( strtol( argument_string, &end_pointer, 10 ), (long) range_min, (long) range_max );
                        if ( *end_pointer != '\0' ) return -1;
                        log_trace( "Argument type unsigned int: %lu", arg->ui );
                        break;
                case ARG_TYPE_FLOAT:
                        arg->f = clamp_range_float( strtof( argument_string, &end_pointer ), (float) range_min, (float) range_max );
                        if ( *end_pointer != '\0' ) return -1;
                        log_trace( "Argument type float: %f", arg->f );
                        break;
                case ARG_TYPE_POINTER:
                        arg->v = strdup( argument_string );
                        if ( !arg->v ) {
                                perror( "strdup failed" );
                                return -1;
                        }
                        log_trace( "Argument type pointer (string): \"%s\", (pointer): %p", argument_string, arg->v );
                        break;
                default:
                        log_error( "Unknown argument type during bind parsing: %d", *arg_type );
                        return -1;
        }
        // @formatter:on

        return 0;
}

// TODO: Split some of these functions into left/right functions, as many accept negative values to do opposite direction actions
// TODO: Make a bind to move both view and client to another monitor
// TODO: Rename some of these functions
// TODO: Add a bind to set theme based off a string value matching the theme name
// TODO: Add a bind to resize a client based off a keyboard bind. Something that would allow you to increment a size in a direction by +/- X
static int parse_bind_function( const char *function_string, enum Argument_Type *arg_type, void ( **function )( const Argument_t * ), long double *range_min, long double *range_max ) {

        const struct {
                const char *name;
                void ( *func )( const Argument_t * );
                enum Argument_Type arg_type;
                const long double range_min, range_max;
        } function_alias_map[ ] = {
                // NOLINTBEGIN(bugprone-integer-division)
                { "center-floating", center_floating, ARG_TYPE_NONE },
                { "client-and-view-to-tag", client_and_view_to_tag, ARG_TYPE_UINT, 1, TAGS_COUNT },
                { "client-and-view-to-tag-bitmask", client_and_view_to_tag_bitmask, ARG_TYPE_INT, -1, TAGMASK },
                { "client-to-monitor", client_to_monitor, ARG_TYPE_INT, -1, 1 },
                { "client-to-tag", client_to_tag, ARG_TYPE_UINT, 1, TAGS_COUNT },
                { "client-to-tag-bitmask", client_to_tag_bitmask, ARG_TYPE_INT, -1, TAGMASK },
                { "exec", spawn, ARG_TYPE_POINTER },
                { "float-and-move", float_and_move, ARG_TYPE_NONE },
                { "focus-monitor", focus_monitor, ARG_TYPE_INT, -1, 1 },
                { "focus-stack", focus_stack, ARG_TYPE_INT, -1, 1 },
                { "increment-all-gaps", increment_gaps, ARG_TYPE_INT, -9999, 9999 },
                { "increment-clients-in-master-area", increment_number_of_clients_in_master_areas, ARG_TYPE_INT, -99, 99 },
                { "increment-inner-gaps", increment_inner_gaps, ARG_TYPE_INT, -9999, 9999 },
                { "increment-inner-horizontal-gaps", increment_inner_horizontal_gaps, ARG_TYPE_INT, -9999, 9999 },
                { "increment-inner-vertical-gaps", increment_inner_vertical_gaps, ARG_TYPE_INT, -9999, 9999 },
                { "increment-outer-gaps", increment_outer_gaps, ARG_TYPE_INT, -9999, 9999 },
                { "increment-outer-horizontal-gaps", increment_outer_horizontal_gaps, ARG_TYPE_INT, -9999, 9999 },
                { "increment-outer-vertical-gaps", increment_outer_vertical_gaps, ARG_TYPE_INT, -9999, 9999 },
                { "increment-theme", increment_theme, ARG_TYPE_INT, -99, 99 },
                { "kill-client", kill_client, ARG_TYPE_NONE },
                { "layout-scroll", layout_scroll, ARG_TYPE_INT, -1, LENGTH( LAYOUTS ) },
                { "move-mouse", move_mouse, ARG_TYPE_NONE },
                { "quit", quit, ARG_TYPE_NONE, 0, 0 },
                { "reload", restart, ARG_TYPE_NONE, 0, 0 },
                { "reset-gaps", set_gaps_default, ARG_TYPE_NONE },
                { "resize-mouse", resize_mouse, ARG_TYPE_NONE },
                { "rotate-stack", rotate_stack, ARG_TYPE_INT, -1, 1 },
                { "set-layout", set_layout_number, ARG_TYPE_UINT, 0, LENGTH( LAYOUTS ) - 1 },
                { "set-master-factor", set_master_factor, ARG_TYPE_FLOAT, -0.95, 0.95 },
                { "shift-both", shift_both, ARG_TYPE_INT, -( TAGS_COUNT - 1 ), TAGS_COUNT - 1 },
                { "shift-client", shift_client, ARG_TYPE_INT, -( TAGS_COUNT - 1 ), TAGS_COUNT - 1 },
                { "shift-view", shift_view, ARG_TYPE_INT, -( TAGS_COUNT - 1 ), TAGS_COUNT - 1 },
                { "toggle-bar", toggle_bar, ARG_TYPE_NONE },
                { "toggle-fake-fullscreen", toggle_fake_fullscreen, ARG_TYPE_NONE },
                { "toggle-floating", toggle_floating, ARG_TYPE_NONE },
                { "toggle-fullscreen", toggle_fullscreen, ARG_TYPE_NONE },
                { "toggle-gaps", toggle_gaps, ARG_TYPE_NONE },
                { "toggle-scratchpad", toggle_scratch, ARG_TYPE_POINTER },
                { "toggle-smart-gaps", toggle_smart_gaps, ARG_TYPE_NONE },
                { "toggle-sticky", toggle_sticky, ARG_TYPE_NONE },
                { "toggle-view-tag", toggle_view_tag, ARG_TYPE_UINT, 1, TAGS_COUNT },
                { "toggle-view-tag-bitmask", toggle_view_tag_bitmask, ARG_TYPE_INT, -1, TAGMASK },
                { "view-tag", view_tag, ARG_TYPE_UINT, 1, TAGS_COUNT },
                { "view-tag-bitmask", view_tag_bitmask, ARG_TYPE_INT, -1, TAGMASK },
                { "view-tag-all-monitors", view_tag_all_monitors, ARG_TYPE_UINT, 1, TAGS_COUNT },
                { "view-tag-all-monitors-bitmask", view_tag_all_monitors_bitmask, ARG_TYPE_INT, -1, TAGMASK },
                { "zoom", zoom, ARG_TYPE_NONE },
                // NOLINTEND(bugprone-integer-division)
        };

        log_trace( "Function being parsed: \"%s\"", function_string );
        for ( int i = 0; i < LENGTH( function_alias_map ); i++ ) {
                if ( strcasecmp( function_string, function_alias_map[ i ].name ) == 0 ) {
                        *function = function_alias_map[ i ].func;
                        *arg_type = function_alias_map[ i ].arg_type;
                        *range_min = function_alias_map[ i ].range_min;
                        *range_max = function_alias_map[ i ].range_max;
                        log_trace( "Function successfully parsed as %p", (void *) function );
                        return 0;
                }
        }

        return -1;
}

static int parse_bind_modifier( const char *modifier_string, unsigned int *modifier ) {

        const struct {
                const char *name;
                unsigned int mask;
        } modifier_alias_map[ ] = {
                { "super", Mod4Mask },
                { "control", ControlMask },
                { "ctrl", ControlMask },
                { "shift", ShiftMask },
                { "alt", Mod1Mask },
                { "caps", LockMask },
                { "capslock", LockMask },
                { "mod1", Mod1Mask },
                { "mod2", Mod2Mask },
                { "mod3", Mod3Mask },
                { "mod4", Mod4Mask },
                { "mod5", Mod5Mask },
        };

        log_trace( "Modifier being parsed: \"%s\"", modifier_string );

        unsigned int found_modifier = 0;
        for ( int i = 0; i < LENGTH( modifier_alias_map ); i++ ) {
                if ( strcasecmp( modifier_string, modifier_alias_map[ i ].name ) == 0 ) {
                        found_modifier = modifier_alias_map[ i ].mask;
                }
        }

        if ( found_modifier == 0 ) return -1;

        log_trace( "Modifier successfully parsed as %u", found_modifier );
        *modifier |= found_modifier;
        return 0;
}

static int parse_buttonbind( const char *buttonbind_string, Button_t *buttonbind, const unsigned int max_keys ) {

        log_debug( "Buttonbind string to parse: \"%s\"", buttonbind_string );

        char buttonbind_string_copy[ 128 ];
        snprintf( buttonbind_string_copy, LENGTH( buttonbind_string_copy ), "%s", buttonbind_string );

        char *modifier_token_list = strtok( buttonbind_string_copy, "," );

        char *click_token = strtok( NULL, "," );
        if ( click_token ) click_token = trim_whitespace( click_token );

        char *function_token = strtok( NULL, "," );
        if ( function_token ) function_token = trim_whitespace( function_token );

        char *argument_token = strtok( NULL, "," );
        if ( argument_token ) argument_token = trim_whitespace( argument_token );

        if ( !modifier_token_list || !function_token || !click_token || modifier_token_list[ 0 ] == '\0' || function_token[ 0 ] == '\0' || click_token[ 0 ] == '\0' ) {
                log_error( "Invalid buttonbind string. Expected format: \"mod+key, click, function, arg (if necessary)\" and got \"%s\"", buttonbind_string );
                return -1;
        }

        // Split `modifier_token_list` into tokens
        unsigned int modifier_token_count = 0;
        char *trimmed_modifier_token_list[ max_keys ];
        memset( trimmed_modifier_token_list, 0, sizeof( trimmed_modifier_token_list ) );
        char *tmp_token = strtok( modifier_token_list, "+" );
        while ( tmp_token && modifier_token_count < max_keys ) {
                char *trimmed = trim_whitespace( tmp_token );
                if ( *trimmed ) {
                        trimmed_modifier_token_list[ modifier_token_count ] = trimmed;
                        modifier_token_count++;
                }
                tmp_token = strtok( NULL, "+" );
        }

        if ( modifier_token_count == 0 ) {
                log_error( "Empty modifier+button field in buttonbind \"%s\"", buttonbind_string );
                return -1;
        }

        if ( modifier_token_count == max_keys && tmp_token ) {
                log_error( "Too many binds (max_keys = %d) in modifier+button field in buttonbind \"%s\"", max_keys, buttonbind_string );
                return -1;
        }

        for ( int i = 0; i < modifier_token_count - 1; i++ ) {
                if ( parse_bind_modifier( trimmed_modifier_token_list[ i ], &buttonbind->modifiers ) ) {
                        log_error( "Invalid modifier \"%s\" in buttonbind \"%s\"", trimmed_modifier_token_list[ i ], buttonbind_string );
                        return -1;
                }
        }

        if ( parse_buttonbind_button( trimmed_modifier_token_list[ modifier_token_count - 1 ], &buttonbind->button ) ) {
                log_error( "Invalid button \"%s\" in buttonbind \"%s\"", function_token, buttonbind_string );
                return -1;
        }

        if ( parse_buttonbind_click( click_token, &buttonbind->click ) ) {
                log_error( "Invalid click \"%s\" in buttonbind \"%s\"", function_token, buttonbind_string );
                return -1;
        }

        long double range_min, range_max;
        if ( parse_bind_function( function_token, &buttonbind->argument_type, &buttonbind->function, &range_min, &range_max ) ) {
                log_error( "Invalid function \"%s\" in buttonbind \"%s\"", function_token, buttonbind_string );
                return -1;
        }

        if ( buttonbind->argument_type != ARG_TYPE_NONE ) {
                if ( parse_bind_argument( argument_token, &buttonbind->argument_type, &buttonbind->argument, range_min, range_max ) ) {
                        log_error( "Invalid argument \"%s\" in buttonbind \"%s\"", argument_token, buttonbind_string );
                        return -1;
                }
        } else {
                log_trace( "Argument type none" );
        }

        return 0;
}

static int parse_buttonbind_button( const char *button_string, unsigned int *button ) {

        const struct {
                const char *name;
                const int button;
        } button_alias_map[ ] = {
                { "leftclick", 1 },
                { "left-click", 1 },
                { "middleclick", 2 },
                { "middle-click", 2 },
                { "rightclick", 3 },
                { "right-click", 3 },
                { "scrollup", 4 },
                { "scroll-up", 4 },
                { "scrolldown", 5 },
                { "scroll-down", 5 },
        };

        log_trace( "Button string to parse: \"%s\"", button_string );
        for ( int i = 0; i < LENGTH( button_alias_map ); i++ ) {
                if ( strcasecmp( button_string, button_alias_map[ i ].name ) == 0 ) {
                        *button = button_alias_map[ i ].button;
                        log_trace( "Button successfully parsed as \"%s\" -> %d", button_alias_map[ i ].name, button_alias_map[ i ].button );
                        return 0;
                }
        }

        errno = 0;
        char *end_pointer = NULL;
        const unsigned long parsed_value = strtoul( button_string, &end_pointer, 10 );

        if ( errno != 0 || end_pointer == button_string || *end_pointer != '\0' ) return -1;
        if ( parsed_value < 1 || parsed_value > 255 ) return -1;

        *button = (unsigned int) parsed_value;

        log_trace( "Button successfully parsed as %d", *button );
        return 0;
}

static int parse_buttonbind_click( const char *click_string, unsigned int *click ) {

        const struct {
                const char *name;
                const enum Clicks click;
        } click_alias_map[ ] = {
                { "tag", CLICK_TAG },
                { "layout", CLICK_LAYOUT_SYMBOL },
                { "status", CLICK_STATUS_TEXT },
                { "title", CLICK_WINDOW_TITLE },
                { "window", CLICK_CLIENT_WINDOW },
                { "desktop", CLICK_DESKTOP },
        };

        log_trace( "Click string to parse: \"%s\"", click_string );
        for ( int i = 0; i < LENGTH( click_alias_map ); i++ ) {
                if ( strcasecmp( click_string, click_alias_map[ i ].name ) == 0 ) {
                        *click = click_alias_map[ i ].click;
                        log_trace( "Click successfully parsed as \"%s\" -> %d", click_alias_map[ i ].name, click_alias_map[ i ].click );
                        return 0;
                }
        }
        return -1;
}

static int parse_buttonbinds_config( const config_t *config, Button_t **buttonbind_config, unsigned int *buttonbind_count, const unsigned int max_keys, bool *default_buttonbinds_loaded ) {

        // I may look at adjusting how memory is allocated and used here. For example,
        // if a bind fails and is assigned. This just leaves empty unused memory. Not
        // the worst, as most users should not have many or any failing keybinds in
        // their config, certainly not enough to cause seriously egregious levels of
        // memory waste, but still something to consider.

        TIMER_CREATE( buttonbinds_parsing_timer );
        TIMER_INITIALIZE( buttonbinds_parsing_timer );

        int failed_buttonbinds_count = 0;
        const config_setting_t *buttonbinds = config_lookup( config, "buttonbinds" );
        if ( buttonbinds != NULL ) {
                *buttonbind_count = config_setting_length( buttonbinds );

                if ( *buttonbind_count == 0 ) {
                        log_warn( "No buttonbinds listed, assigning minimal default buttonbinds and exiting buttonbind parsing" );
                        load_default_buttonbind_config( buttonbind_config, buttonbind_count, default_buttonbinds_loaded );
                        return 1;
                }

                log_debug( "Buttonbinds detected: %d", *buttonbind_count );

                *buttonbind_config = ecalloc( *buttonbind_count, sizeof( Button_t ) );

                const config_setting_t *buttonbind = NULL;

                for ( int i = 0; i < *buttonbind_count; i++ ) {
                        buttonbind = config_setting_get_elem( buttonbinds, i );

                        if ( buttonbind == NULL ) {
                                log_error( "Buttonbind element %d returned null, unable to parse", i + 1 );
                                failed_buttonbinds_count++;
                                continue;
                        }

                        if ( parse_buttonbind( config_setting_get_string( buttonbind ), &( *buttonbind_config )[ i ], max_keys ) == -1 ) {
                                ( *buttonbind_config )[ i ].button = 0;
                                failed_buttonbinds_count++;
                        }

                        buttonbind = NULL;
                }
                log_debug( "%d buttonbinds failed to be parsed", failed_buttonbinds_count );
        } else {
                log_error( "Problem reading config value \"buttonbinds\": Not found" );
                log_warn( "Default buttonbinds will be loaded. It is recommended you fix the config and reload jdwm" );
        }

        TIMER_CHECKPOINT( buttonbinds_parsing_timer, "Keybinds config parsing" );

        return failed_buttonbinds_count;
}

static int parse_generic_settings( const config_t *config, Master_Config_t *master_config ) {

        TIMER_CREATE( generic_settings_parsing_timer );
        TIMER_INITIALIZE( generic_settings_parsing_timer );

        // Maybe come back to these range values in the future. Things like padding
        // should maybe be limited using screen size, or operate on a % of screen size.
        // Needs more thought.

        enum Setting_Type {
                TYPE_BOOL,
                TYPE_INT,
                TYPE_UINT,
                TYPE_COLOR,
                TYPE_STRING
        };

        const struct {
                const char *name;
                void *value;
                const enum Setting_Type type;
                const bool optional;
                const long double range_min, range_max;
        } setting_map[ ] = {

                // Bar settings
                { "show-bar-by-default", &master_config->bar_config.show_by_default, TYPE_BOOL, true },
                { "show-system-tray", &master_config->bar_config.show_system_tray, TYPE_BOOL, true },
                { "show-status-text", &master_config->bar_config.show_status_text, TYPE_BOOL, true },
                { "show-tags", &master_config->bar_config.show_tags, TYPE_BOOL, true },
                { "show-client-title", &master_config->bar_config.show_client_title, TYPE_BOOL, true },
                { "show-layout-symbol", &master_config->bar_config.show_layout_symbol, TYPE_BOOL, true },
                { "show-floating-indicator", &master_config->bar_config.show_floating_indicator, TYPE_BOOL, true },
                { "show-tag-underline", &master_config->bar_config.show_tag_underline, TYPE_BOOL, true },
                { "enable-top-bar", &master_config->bar_config.enable_top_bar, TYPE_BOOL, true },
                { "enable-centered-client-title", &master_config->bar_config.enable_centered_client_title, TYPE_BOOL, true },
                { "vertical-bar-padding", &master_config->bar_config.vertical_bar_padding, TYPE_INT, true, -9999, 9999 },
                { "horizontal-bar-padding", &master_config->bar_config.horizontal_bar_padding, TYPE_INT, true, -9999, 9999 },
                { "vertical-bar-text-padding", &master_config->bar_config.vertical_bar_text_padding, TYPE_INT, true, -9999, 9999 },
                { "horizontal-bar-text-padding", &master_config->bar_config.horizontal_bar_text_padding, TYPE_INT, true, -9999, 9999 },
                { "underline-thickness", &master_config->bar_config.underline_thickness, TYPE_UINT, true, 0, 9999 },
                { "underline-vertical-padding", &master_config->bar_config.underline_vertical_padding, TYPE_INT, true, -9999, 9999 },
                { "underline-horizontal-padding", &master_config->bar_config.underline_horizontal_padding, TYPE_INT, true, -9999, 9999 },
                { "system-tray-padding", &master_config->bar_config.system_tray_padding, TYPE_INT, true, -9999, 9999 },
                { "system-tray-pinning", &master_config->bar_config.system_tray_pinning, TYPE_UINT, true, 0, 99 },
                { "client-icon-size", &master_config->bar_config.client_icon_size, TYPE_UINT, true, 0, 999 },
                { "client-icon-padding", &master_config->bar_config.client_icon_padding, TYPE_INT, true, -9999, 9999 },

                // Layout Settings
                { "force-vertical-split", &master_config->layout_config.force_vertical_split, TYPE_BOOL, true },
                { "respect-size-hints", &master_config->layout_config.respect_resize_hints, TYPE_BOOL, true },
                { "smart-gaps", &master_config->layout_config.smart_gaps, TYPE_BOOL, true },
                { "default-number-of-clients-in-master-area", &master_config->layout_config.default_number_of_clients_in_master_area, TYPE_UINT, true, 1, 99 },
                { "default-master-factor", &master_config->layout_config.default_master_factor, TYPE_UINT, true, 1, 99 },
                { "default-inner-horizontal-gap", &master_config->layout_config.default_inner_horizontal_gap, TYPE_UINT, true, 0, 9999 },
                { "default-inner-vertical-gap", &master_config->layout_config.default_inner_vertical_gap, TYPE_UINT, true, 0, 9999 },
                { "default-outer-horizontal-gap", &master_config->layout_config.default_outer_horizontal_gap, TYPE_UINT, true, 0, 9999 },
                { "default-outer-vertical-gap", &master_config->layout_config.default_outer_vertical_gap, TYPE_UINT, true, 0, 9999 },

                // Window Settings
                { "normal-border-width", &master_config->window_config.normal_border_width, TYPE_INT, true, 0, 999 },
                { "floating-border-width", &master_config->window_config.floating_border_width, TYPE_INT, true, 0, 999 },
                { "snapping-pixel-distance", &master_config->window_config.snapping_pixel_distance, TYPE_INT, true, 1, 999 },
                { "enable-raise-floating-window-on-focus", &master_config->window_config.enable_raise_floating_windows_on_focus, TYPE_BOOL, true },

                // Advanced
                { "enable-system-tray-pin-to-first-monitor-on-fail", &master_config->enable_system_tray_pin_to_first_monitor_on_fail, TYPE_BOOL, true },
                { "max-keys", &master_config->max_keys, TYPE_UINT, true, 1, 10 },
                { "mouse-refresh-rate", &master_config->mouse_refresh_rate, TYPE_UINT, true, 1, 999 },
        };

        log_debug( "Generic settings available: %lu", LENGTH( setting_map ) );

        int settings_failed_count = 0;
        for ( int i = 0; i < LENGTH( setting_map ); ++i ) {
                switch ( setting_map[ i ].type ) {
                        case TYPE_BOOL:
                                settings_failed_count -= libconfig_lookup_bool( config, setting_map[ i ].name, setting_map[ i ].value, setting_map[ i ].optional );
                                break;
                        case TYPE_INT:
                                settings_failed_count -= libconfig_lookup_int( config, setting_map[ i ].name, setting_map[ i ].value, setting_map[ i ].optional, (int) setting_map[ i ].range_min,
                                                                               (int) setting_map[ i ].range_max );
                                break;
                        case TYPE_UINT:
                                settings_failed_count -= libconfig_lookup_uint( config, setting_map[ i ].name, setting_map[ i ].value, setting_map[ i ].optional,
                                                                                (unsigned int) setting_map[ i ].range_min, (unsigned int) setting_map[ i ].range_max );
                                break;
                        case TYPE_COLOR:
                                settings_failed_count -= libconfig_lookup_color( config, setting_map[ i ].name, setting_map[ i ].value, setting_map[ i ].optional );
                                break;
                        case TYPE_STRING:
                                settings_failed_count -= libconfig_lookup_string( config, setting_map[ i ].name, setting_map[ i ].value, setting_map[ i ].optional );
                                break;
                }
        }

        log_debug( "%d generic settings failed to be parsed", settings_failed_count );

        TIMER_CHECKPOINT( generic_settings_parsing_timer, "Parsing generic settings config" );

        return settings_failed_count;
}

static int parse_keybind( const char *keybind_string, Key_t *keybind, const unsigned int max_keys ) {

        log_debug( "Keybind string to parse: \"%s\"", keybind_string );

        char keybind_string_copy[ 128 ];
        snprintf( keybind_string_copy, LENGTH( keybind_string_copy ), "%s", keybind_string );

        char *modifier_token_list = strtok( keybind_string_copy, "," );

        char *function_token = strtok( NULL, "," );
        if ( function_token ) function_token = trim_whitespace( function_token );

        char *argument_token = strtok( NULL, "," );
        if ( argument_token ) argument_token = trim_whitespace( argument_token );

        if ( !modifier_token_list || !function_token || modifier_token_list[ 0 ] == '\0' || function_token[ 0 ] == '\0' ) {
                log_error( "Invalid keybind string. Expected format: \"mod+key, function, arg (if necessary)\" and got \"%s\"", keybind_string );
                return -1;
        }

        long double range_min, range_max;
        if ( parse_bind_function( function_token, &keybind->argument_type, &keybind->function, &range_min, &range_max ) ) {
                log_error( "Invalid function \"%s\" in keybind \"%s\"", function_token, keybind_string );
                return -1;
        }

        if ( keybind->argument_type != ARG_TYPE_NONE ) {
                if ( parse_bind_argument( argument_token, &keybind->argument_type, &keybind->argument, range_min, range_max ) ) {
                        log_error( "Invalid argument \"%s\" in keybind \"%s\"", argument_token, keybind_string );
                        return -1;
                }
        } else {
                log_trace( "Argument type none" );
        }

        // Split `modifier_token_list` into tokens
        unsigned int modifier_token_count = 0;
        char *trimmed_modifier_token_list[ max_keys ];
        memset( trimmed_modifier_token_list, 0, sizeof( trimmed_modifier_token_list ) );
        char *tmp_token = strtok( modifier_token_list, "+" );
        while ( tmp_token && modifier_token_count < max_keys ) {
                char *trimmed = trim_whitespace( tmp_token );
                if ( *trimmed ) {
                        trimmed_modifier_token_list[ modifier_token_count ] = trimmed;
                        modifier_token_count++;
                }
                tmp_token = strtok( NULL, "+" );
        }

        if ( modifier_token_count == 0 ) {
                log_error( "Empty modifier+key field in keybind \"%s\"", keybind_string );
                return -1;
        }

        if ( modifier_token_count == max_keys && tmp_token ) {
                log_error( "Too many binds (max_keys = %d) in modifier+key field in keybind \"%s\"", max_keys, keybind_string );
                return -1;
        }

        for ( int i = 0; i < modifier_token_count - 1; i++ ) {
                if ( parse_bind_modifier( trimmed_modifier_token_list[ i ], &keybind->modifiers ) ) {
                        log_error( "Invalid modifier \"%s\" in keybind \"%s\"", trimmed_modifier_token_list[ i ], keybind_string );
                        return -1;
                }
        }

        if ( parse_keybind_keysym( trimmed_modifier_token_list[ modifier_token_count - 1 ], &keybind->keysym ) ) {
                log_error( "Invalid keysym \"%s\" in keybind \"%s\"", trimmed_modifier_token_list[ modifier_token_count - 1 ], keybind_string );
                return -1;
        }

        return 0;
}

static int parse_keybind_keysym( const char *keysym_string, KeySym *keysym ) {

        log_trace( "Keysym being parsed: \"%s\"", keysym_string );

        *keysym = XStringToKeysym( keysym_string );
        if ( *keysym == NoSymbol ) return -1;

        KeySym dummy = 0; // Unused, just needs to exist to satisfy compiler
        XConvertCase( *keysym, keysym, &dummy );

        log_trace( "Keysym successfully parsed as parsed: \"%s\" -> 0x%lx", XKeysymToString( *keysym ), *keysym );
        return 0;
}

static int parse_keybinds_config( const config_t *config, Key_t **keybind_config, unsigned int *keybinds_count, const unsigned int max_keys, bool *default_keybinds_loaded ) {

        // I may look at adjusting how memory is allocated and used here. For example,
        // if a bind fails and is assigned. This just leaves empty unused memory. Not
        // the worst, as most users should not have many or any failing keybinds in
        // their config, certainly not enough to cause seriously egregious levels of
        // memory waste, but still something to consider.

        TIMER_CREATE( keybinds_parsing_timer );
        TIMER_INITIALIZE( keybinds_parsing_timer );

        int failed_keybinds = 0;
        const config_setting_t *keybinds = config_lookup( config, "keybinds" );
        if ( keybinds != NULL ) {
                *keybinds_count = config_setting_length( keybinds );

                if ( *keybinds_count == 0 ) {
                        log_warn( "No keybinds listed, assigning minimal default keybinds and exiting keybinds parsing" );
                        load_default_keybind_config( keybind_config, keybinds_count, default_keybinds_loaded );
                        return 1;
                }

                log_debug( "Keybinds detected: %d", *keybinds_count );

                *keybind_config = ecalloc( *keybinds_count, sizeof( Key_t ) );
                const config_setting_t *keybind = NULL;
                for ( int i = 0; i < *keybinds_count; i++ ) {
                        keybind = config_setting_get_elem( keybinds, i );

                        if ( keybind == NULL ) {
                                log_error( "Keybind element %d returned null, unable to parse", i + 1 );
                                failed_keybinds++;
                                continue;
                        }

                        if ( parse_keybind( config_setting_get_string( keybind ), &( *keybind_config )[ i ], max_keys ) == -1 ) {
                                ( *keybind_config )[ i ].keysym = NoSymbol;
                                failed_keybinds++;
                        }

                        keybind = NULL;
                }
                log_debug( "%d keybinds failed to be parsed", failed_keybinds );
        } else {
                log_error( "Problem reading config value \"keybinds\": Not found" );
                log_warn( "Default keybinds will be loaded. It is recommended you fix the config and reload jdwm" );
        }

        TIMER_CHECKPOINT( keybinds_parsing_timer, "Keybinds config parsing" );

        return failed_keybinds;
}

static int parse_logger_settings( const config_t *config ) {

        int failed_loggers_count = 0;
        int failed_loggers_elements_count = 0;

        TIMER_CREATE( logger_parsing_timer );
        TIMER_INITIALIZE( logger_parsing_timer );

        if ( !g_cli_flags[ FLAG_CUSTOM_LOG_LEVEL ] ) {
                failed_loggers_elements_count -= libconfig_lookup_uint( config, "default-log-level", &g_logging_level, true, LOG_TRACE, LOG_FATAL );
                log_info( "Default log level loaded from config file as %s", LOG_LEVEL_STRINGS[ g_logging_level ] );
                failed_loggers_elements_count -= logger_set_log_level( DEFAULT_LOGGER_NAME, g_logging_level );
        } else {
                log_info( "Logging level overridden from cli. Skipping parsing \"default-log-level\"" );
        }

        const config_setting_t *loggers = config_lookup( config, "loggers" );
        if ( loggers != NULL ) {

                const char *name_string = NULL;
                const char *output_string = NULL;
                const char *filemode_string = NULL;
                int log_level = 0;
                bool use_color = 0;

                const config_setting_t *logger = NULL;
                for ( int i = 0; i < config_setting_length( loggers ); i++ ) {
                        logger = config_setting_get_elem( loggers, i );
                        if ( logger != NULL ) {

                                int immediate_failed_elements = 0;

                                immediate_failed_elements -= libconfig_setting_lookup_string( logger, "name", &name_string, false );
                                immediate_failed_elements -= libconfig_setting_lookup_int( logger, "log-level", &log_level, false, -1, LOG_FATAL );
                                immediate_failed_elements -= libconfig_setting_lookup_string( logger, "output", &output_string, false );

                                if ( log_level < 0 ) {
                                        log_debug( "Parsed log level for logger named \"%s\" (i=%d) was %d, setting log level to the global log level %d", name_string, i, log_level, g_logging_level );
                                        log_level = (int) g_logging_level;
                                } else if ( log_level > LOG_FATAL ) {
                                        log_debug( "Parsed log level for logger named \"%s\" (i=%d) was >%d, setting log level to %d (fatal)", name_string, i, LOG_FATAL, LOG_FATAL );
                                        log_level = LOG_FATAL;
                                }

                                if ( strcasecmp( output_string, "stderr" ) == 0 ) {
                                        immediate_failed_elements -= libconfig_setting_lookup_bool( logger, "use-color", &use_color, false );
                                        if ( immediate_failed_elements ) goto logger_failed;
                                        logger_add_stream_handler( name_string, stderr, log_level, false, use_color );
                                        continue;
                                }

                                if ( strcasecmp( output_string, "stdout" ) == 0 ) {
                                        immediate_failed_elements -= libconfig_setting_lookup_bool( logger, "use-color", &use_color, false );
                                        if ( immediate_failed_elements ) goto logger_failed;
                                        logger_add_stream_handler( name_string, stdout, log_level, false, use_color );
                                        continue;
                                }

                                immediate_failed_elements -= libconfig_setting_lookup_string( logger, "filemode", &filemode_string, false );
                                if ( strcasecmp( filemode_string, "w" ) != 0 && strcasecmp( filemode_string, "a" ) != 0 ) immediate_failed_elements++;

                                if ( immediate_failed_elements ) goto logger_failed;
                                logger_add_file_handler( name_string, output_string, filemode_string, log_level, false, use_color );
                                continue;

                                // @formatter:off
                                logger_failed:
                                        log_warn( "Unable to create logger (index = %d) from config, as %d elements were invalid", i, immediate_failed_elements );
                                        failed_loggers_elements_count += immediate_failed_elements;
                                // @formatter:on
                        } else {
                                log_error( "Logger %d returned null, unable to parse", i + 1 );
                                failed_loggers_count++;
                        }
                }
        } else {
                log_error( "Problem reading config value \"loggers\": Not found" );
        }

        log_debug( "%d loggers failed to be parsed", failed_loggers_count );
        log_debug( "Of those loggers, %d logger elements failed to be parsed", failed_loggers_elements_count );

        TIMER_CHECKPOINT( logger_parsing_timer, "Parsing loggers config" );

        return failed_loggers_count + failed_loggers_elements_count;
}

static int parse_rules_string( const char *input_string, char *output_string, const int output_string_size ) {

        if ( input_string == NULL ) return -1;

        if ( strcasecmp( input_string, "null" ) == 0 ) {
                output_string[ 0 ] = '\0';
                return 0;
        }

        snprintf( output_string, output_string_size, "%s", input_string );

        return 0;
}

static int parse_rules_config( const config_t *config, Rule_t **rules_config, unsigned int *rules_count ) {

        TIMER_CREATE( rules_parsing_timer );
        TIMER_INITIALIZE( rules_parsing_timer );

        int failed_rules_count = 0;
        int failed_rules_elements_count = 0;

        const config_setting_t *rules = config_lookup( config, "rules" );
        if ( rules != NULL ) {
                *rules_count = config_setting_length( rules );

                if ( *rules_count == 0 ) {
                        log_warn( "No rules listed, exiting rules parsing" );
                        return 1;
                }

                log_debug( "Rules detected: %d", *rules_count );

                *rules_config = ecalloc( *rules_count, sizeof( Rule_t ) );

                // Set some sane default values
                for ( int i = 0; i < *rules_count; i++ ) {
                        ( *rules_config )[ i ].class[ 0 ] = '\0';
                        ( *rules_config )[ i ].instance[ 0 ] = '\0';
                        ( *rules_config )[ i ].title[ 0 ] = '\0';
                        ( *rules_config )[ i ].tags = 0;
                        ( *rules_config )[ i ].monitor = -1;
                        ( *rules_config )[ i ].is_floating = false;
                        ( *rules_config )[ i ].ignore_transient = false;
                        ( *rules_config )[ i ].unmanaged = false;
                }

                const char *tmp_string = NULL;
                const config_setting_t *rule = NULL;

                for ( int i = 0; i < *rules_count; i++ ) {
                        rule = config_setting_get_elem( rules, i );
                        if ( rule != NULL ) {

                                libconfig_setting_lookup_string( rule, "class", &tmp_string, false );
                                if ( parse_rules_string( tmp_string, ( *rules_config )[ i ].class, sizeof( ( *rules_config )[ i ].class ) ) ) {
                                        log_error( "Problem parsing \"class\" value of rule %d", i + 1 );
                                        failed_rules_elements_count++;
                                }

                                libconfig_setting_lookup_string( rule, "instance", &tmp_string, false );
                                if ( parse_rules_string( tmp_string, ( *rules_config )[ i ].instance, sizeof( ( *rules_config )[ i ].instance ) ) ) {
                                        log_error( "Problem parsing \"instance\" value of rule %d", i + 1 );
                                        failed_rules_elements_count++;
                                }

                                libconfig_setting_lookup_string( rule, "title", &tmp_string, false );
                                if ( parse_rules_string( tmp_string, ( *rules_config )[ i ].title, sizeof( ( *rules_config )[ i ].title ) ) ) {
                                        log_error( "Problem parsing \"title\" value of rule %d", i + 1 );
                                        failed_rules_elements_count++;
                                }

                                failed_rules_elements_count -= libconfig_setting_lookup_uint( rule, "tag-mask", &( *rules_config )[ i ].tags, false, 0, TAGMASK );
                                failed_rules_elements_count -= libconfig_setting_lookup_int( rule, "monitor", &( *rules_config )[ i ].monitor, false, -1, 99 );
                                failed_rules_elements_count -= libconfig_setting_lookup_bool( rule, "floating", &( *rules_config )[ i ].is_floating, false );
                                failed_rules_elements_count -= libconfig_setting_lookup_bool( rule, "ignore-transient", &( *rules_config )[ i ].ignore_transient, false );
                                failed_rules_elements_count -= libconfig_setting_lookup_bool( rule, "unmanaged", &( *rules_config )[ i ].unmanaged, false );
                        } else {
                                log_error( "Rule %d returned null, unable to parse", i + 1 );
                                failed_rules_count++;
                        }
                }
        } else {
                log_error( "Problem reading config value \"rules\": Not found" );
                return 1;
        }

        log_debug( "%d rules failed to be parsed", failed_rules_count );
        log_debug( "Of those rules, %d rule elements failed to be parsed", failed_rules_elements_count );

        TIMER_CHECKPOINT( rules_parsing_timer, "Rules config parsing" );

        return failed_rules_count + failed_rules_elements_count;
}

static int parse_tags_config( const config_t *config, Bar_Config_t *bar_config ) {

        TIMER_CREATE( tags_parsing_timer );
        TIMER_INITIALIZE( tags_parsing_timer );

        int tags_failed_count = 0;

        const config_setting_t *tags = config_lookup( config, "tag-names" );
        if ( tags != NULL ) {
                int i = 0;
                const char *tag_name = NULL;
                const int tags_count = config_setting_length( tags );

                if ( tags_count == 0 ) {
                        log_warn( "No tag names detected while parsing config, default tag names will be used" );
                        return 1;
                }

                log_debug( "Tags detected: %d", tags_count );

                if ( tags_count > 9 ) {
                        log_warn( "More than 9 tag names detected (%d) while parsing config, only the first 9 will be used", tags_count );
                } else if ( tags_count < 9 ) {
                        log_warn( "Less than 9 tags detected (%d) while parsing config, default tags will be used to fill the remainder", tags_count );
                }

                for ( i = 0; i < tags_count && i < 9; i++ ) {
                        tag_name = config_setting_get_string_elem( tags, i );

                        if ( tag_name == NULL ) {
                                log_error( "Problem reading tag array element %d: Value doesn't exist or isn't a string", i + 1 );
                                tags_failed_count++;
                                continue;
                        }

                        SAFE_FREE( bar_config->tags[ i ] );
                        bar_config->tags[ i ] = strdup( tag_name );
                        if ( bar_config->tags[ i ] == NULL ) {
                                log_error( "strdup failed while copying parsed tag %d", i );
                                tags_failed_count++;
                                continue;
                        }
                }
        } else {
                log_error( "Problem reading config value \"tag-names\": Not found" );
                return 1;
        }

        log_debug( "%d tags failed to be parsed", tags_failed_count );

        TIMER_CHECKPOINT( tags_parsing_timer, "Tags config parsing" );

        return tags_failed_count;
}

static int parse_theme( config_setting_t *theme, Theme_Config_t *theme_config, const int theme_index ) {

        const char *tmp = NULL;
        int theme_elements_failed_count = 0;

        if ( !libconfig_setting_lookup_string( theme, "name", &tmp, false ) ) snprintf( theme_config->name, LENGTH( theme_config->name ), "%s", tmp );
        if ( !libconfig_setting_lookup_string( theme, "font", &tmp, false ) ) snprintf( theme_config->font, LENGTH( theme_config->font ), "%s", tmp );

        theme_elements_failed_count -= libconfig_setting_lookup_color( theme, "system-tray", &theme_config->color_systray, false );

        theme_elements_failed_count -= parse_theme_client( theme, "client-selected", theme_config->color_client_selected, theme_index );
        theme_elements_failed_count -= parse_theme_client( theme, "client-unselected", theme_config->color_client_unselected, theme_index );

        theme_elements_failed_count -= parse_theme_bar_section( theme, "layout", theme_config->color_layout, theme_index );
        theme_elements_failed_count -= parse_theme_bar_section( theme, "statusbar", theme_config->color_statusbar, theme_index );
        theme_elements_failed_count -= parse_theme_bar_section( theme, "tag-selected", theme_config->color_tag_selected, theme_index );
        theme_elements_failed_count -= parse_theme_bar_section( theme, "tag-unselected", theme_config->color_tag_unselected, theme_index );
        theme_elements_failed_count -= parse_theme_bar_section( theme, "titlebar-selected", theme_config->color_titlebar_selected, theme_index );
        theme_elements_failed_count -= parse_theme_bar_section( theme, "titlebar-unselected", theme_config->color_titlebar_unselected, theme_index );

        return theme_elements_failed_count;
}

static int parse_theme_client( config_setting_t *theme, const char *path, XftColor *client_theme, const int theme_index ) {

        int failed_settings_count = 0;
        const config_setting_t *tmp_config_setting = config_setting_lookup( theme, path );

        if ( tmp_config_setting != NULL ) {
                failed_settings_count -= libconfig_setting_lookup_color( tmp_config_setting, "normal-border", &client_theme[ NORMAL_BORDER ], false );
                failed_settings_count -= libconfig_setting_lookup_color( tmp_config_setting, "floating-border", &client_theme[ FLOATING_BORDER ], false );
        } else {
                log_error( "Problem reading config value \"%s\" in theme %d: Not found", path, theme_index + 1 );
                return -1;
        }

        log_debug( "%d theme elements failed to be parsed in setting \"%s\" in theme %d's client sub-structure", failed_settings_count, path, theme_index + 1 );

        return failed_settings_count;
}

static int parse_theme_bar_section( config_setting_t *theme, const char *path, XftColor *client_theme, const int theme_index ) {

        int failed_settings_count = 0;
        const config_setting_t *tmp_config_setting = config_setting_lookup( theme, path );

        if ( tmp_config_setting != NULL ) {
                failed_settings_count -= libconfig_setting_lookup_color( tmp_config_setting, "text", &client_theme[ FOREGROUND ], false );
                failed_settings_count -= libconfig_setting_lookup_color( tmp_config_setting, "background", &client_theme[ BACKGROUND ], false );
        } else {
                log_error( "Problem reading config value \"%s\" in theme %d: Not found", path, theme_index + 1 );
                return -1;
        }

        log_debug( "%d theme elements failed to be parsed in setting \"%s\" in theme %d's bar section sub-structure", failed_settings_count, path, theme_index + 1 );

        return failed_settings_count;
}

static int parse_themes_config( const config_t *config, Theme_Config_t **themes_config, unsigned int *themes_count ) {

        TIMER_CREATE( themes_parsing_timer );
        TIMER_INITIALIZE( themes_parsing_timer );

        int failed_themes_count = 0;
        int failed_theme_elements_count = 0;
        const config_setting_t *themes = config_lookup( config, "themes" );

        if ( themes != NULL ) {

                const int detected_theme_count = config_setting_length( themes );

                if ( detected_theme_count == 0 ) {
                        log_error( "Problem reading config value \"themes\": Not themes provided" );
                        log_warn( "Default theme will be loaded" );
                        return 1;
                }

                *themes_count = detected_theme_count;

                log_debug( "Themes detected: %d", *themes_count );

                // Protect in rare case size the array is too large.
                // 99.9% chance this would never happen, but doesn't hurt to guard.
                if ( sizeof( Theme_Config_t ) > SIZE_MAX / *themes_count ) {
                        log_fatal( "Integer overflow when allocating array theme config array. nmemb = %zu, size = %zu", *themes_count, sizeof( Theme_Config_t ) );
                }

                errno = 0;
                Theme_Config_t *resized_array = realloc( *themes_config, *themes_count * sizeof( Theme_Config_t ) );

                if ( !resized_array ) {
                        log_fatal( "realloc: size = %d: %s", *themes_count * sizeof( Theme_Config_t ), strerror( errno ) );
                }

                *themes_config = resized_array;

                // Zero all elements of themes_config array after first theme.
                // First theme was already allocated with ecalloc().
                if ( *themes_count > 1 ) {
                        memset( &( *themes_config )[ 1 ], 0, ( *themes_count - 1 ) * sizeof( Theme_Config_t ) );
                }

                config_setting_t *theme = NULL;

                for ( int i = 0; i < *themes_count; i++ ) {

                        theme = config_setting_get_elem( themes, i );

                        if ( theme == NULL ) {
                                log_error( "Theme %d returned null, unable to parse", i + 1 );
                                failed_themes_count++;
                                continue;
                        }

                        failed_theme_elements_count += parse_theme( theme, &( *themes_config )[ i ], i );
                        log_debug( "%d elements failed to be parsed in theme number %d", failed_theme_elements_count, i + 1 );
                }

                log_debug( "%d themes failed to be parsed", failed_themes_count );

        } else {
                log_error( "Problem reading config value \"themes\": Not found" );
                log_warn( "Default theme \"%s\" will be loaded instead", ( *themes_config )[ g_theme_index ].name );
                return 1;
        }

        const char *buffer = NULL;
        if ( !libconfig_lookup_string( config, "default-theme", &buffer, true ) ) {
                bool found_match = false;

                for ( int i = 0; i < *themes_count; i++ ) {
                        if ( !strcasecmp( ( *themes_config )[ i ].name, buffer ) ) {
                                log_debug( "Theme index is set to %d", i );
                                g_theme_index = i;
                                found_match = true;
                                break;
                        }
                }

                if ( !found_match ) {
                        log_warn( "No theme found with the name \"%s\". Using the first theme, named \"%s\", instead", buffer, ( *themes_config )[ g_theme_index ].name );
                }
        } else {
                log_warn( "No default theme specified. First theme in list, named \"%s\", will be used", themes_config[0]->name );
        }

        log_info( "Theme to be used: \"%s\"", ( *themes_config )[ g_theme_index ].name );

        TIMER_CHECKPOINT( themes_parsing_timer, "Themes config parsing" );

        return failed_themes_count + failed_theme_elements_count;
}

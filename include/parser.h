#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include "common.h"

enum Argument_Type {
        ARG_TYPE_NONE,
        ARG_TYPE_INT,
        ARG_TYPE_UINT,
        ARG_TYPE_FLOAT,
        ARG_TYPE_POINTER,
};

enum Clicks {
        CLICK_TAG,
        CLICK_LAYOUT_SYMBOL,
        CLICK_STATUS_TEXT,
        CLICK_WINDOW_TITLE,
        CLICK_CLIENT_WINDOW,
        CLICK_DESKTOP,
        CLICK_LAST
};

typedef struct Bar_Config {

        // Bar Feature Toggles
        bool show_by_default;
        bool show_status_text;
        bool show_tags;
        bool show_client_title;
        bool show_layout_symbol;
        bool show_floating_indicator;
        bool show_tag_underline;
        bool enable_top_bar;
        bool enable_centered_client_title;
        bool show_system_tray;

        // Generic Paddings
        int vertical_bar_padding, horizontal_bar_padding;
        int vertical_bar_text_padding, horizontal_bar_text_padding;

        // Tag Underline
        int underline_thickness;
        int underline_vertical_padding, underline_horizontal_padding;

        // System Tray
        int system_tray_padding;
        unsigned int system_tray_pinning;

        // Client's Icon
        int client_icon_padding;
        unsigned int client_icon_size;

        // Misc

        // TODO: Allocate completely dynamically.  It would also allow for the number of tags to be defined
        // in the configuration file. NOTE: If you do this, you will also need to consider the tag masks size,
        // uint is only guaranteed to be 16 bits or larger, would need something like u_int64_t or unsigned long long.
        char *tags[ 9 ];
} Bar_Config_t;

typedef struct Layout_Config {

        // Toggles
        bool force_vertical_split;
        bool respect_resize_hints;
        bool smart_gaps;

        // Default Gaps
        int default_number_of_clients_in_master_area;
        int default_master_factor;
        int default_inner_horizontal_gap;
        int default_inner_vertical_gap;
        int default_outer_horizontal_gap;
        int default_outer_vertical_gap;
} Layout_Config_t;

typedef struct Window_Config {
        char scratchpad_name[ 128 ];
        int normal_border_width, floating_border_width;
        int snapping_pixel_distance;
        bool enable_raise_floating_windows_on_focus;
} Window_Config_t;

typedef struct Theme_Config {
        char name[ 128 ];
        char font[ 128 ];
        XftColor color_systray;
        XftColor color_client_selected[ 2 ];
        XftColor color_client_unselected[ 2 ];
        XftColor color_layout[ 2 ];
        XftColor color_statusbar[ 2 ];
        XftColor color_tag_selected[ 2 ];
        XftColor color_tag_unselected[ 2 ];
        XftColor color_titlebar_selected[ 2 ];
        XftColor color_titlebar_unselected[ 2 ];
} Theme_Config_t;

typedef struct Rule {
        char class[ 128 ];
        char instance[ 128 ];
        char title[ 128 ];
        unsigned int tags;
        int monitor;
        bool is_floating;
        bool ignore_transient;
        bool unmanaged;
} Rule_t;

typedef struct Key {
        unsigned int modifiers;
        KeySym keysym;
        void ( *function )( const Argument_t * );
        Argument_t argument;
        enum Argument_Type argument_type;
} Key_t;

typedef struct Button {
        unsigned int click;
        unsigned int modifiers;
        unsigned int button;
        void ( *function )( const Argument_t *argument );
        Argument_t argument;
        enum Argument_Type argument_type;
} Button_t;

// TODO: Adjust to optimize padding and improve readability
typedef struct Master_Config {
        bool fallback_config_loaded;
        bool default_keybinds_loaded, default_buttonbinds_loaded;
        bool enable_system_tray_pin_to_first_monitor_on_fail;

        unsigned int max_keys, mouse_refresh_rate;

        char *config_filepath;

        Bar_Config_t bar_config;
        Layout_Config_t layout_config;
        Window_Config_t window_config;

        unsigned int themes_count;
        Theme_Config_t *themes;

        unsigned int rules_count;
        Rule_t *rules;

        unsigned int keybinds_count;
        Key_t *keybinds;

        unsigned int buttonbinds_count;
        Button_t *buttonbinds;
} Master_Config_t;

extern void config_cleanup( Master_Config_t *master_config );
extern int parse_config( Master_Config_t *master_config );

extern Master_Config_t g_master_config;

#endif /* CONFIG_H_ */

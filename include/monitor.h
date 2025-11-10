#ifndef MONITOR_H_
#define MONITOR_H_

#include <X11/X.h>
#include "common.h"
#include "tags.h"

typedef struct Monitor {
        int monitor_number;
        int bar_y;
        int monitor_x, monitor_y, monitor_width, monitor_height;
        int window_x, window_y, window_width, window_height;
        struct Client *selected_client, *client_list, *client_stack;
        struct Monitor *next_monitor;
        Window bar_window;
        unsigned int selected_tags, tag_set[ 2 ];
        unsigned int current_tag_index;
        struct Tag tags[ TAGS_COUNT ];
} Monitor_t;

extern void arrange( Monitor_t *monitor );
extern void arrange_monitor( Monitor_t *monitor );
extern void cleanup_monitor( Monitor_t *monitor );
extern Monitor_t *create_monitor( void );
extern Monitor_t *direction_to_next_monitor( int direction );
extern void focus_monitor( const Argument_t *argument );
extern void get_facts( const Monitor_t *monitor, int master_size, int stack_size, double *master_factor, double *stack_factor, int *master_remainder, int *stack_remainder );
extern Monitor_t *rectangle_to_monitor( int x, int y, int width, int height );
extern void restack( Monitor_t *monitor );
extern void set_master_factor( const Argument_t *argument );
extern int update_geometry( void );
extern Monitor_t *window_to_monitor( Window window );

extern Monitor_t *g_monitors, *g_selected_monitor, *g_last_selected_monitor;

#endif /* MONITOR_H_ */
#ifndef TAG_H_
#define TAG_H_

#include <stdbool.h>
#include "common.h"
#include "parser.h"

#define TAGMASK ( ( 1 << TAGS_COUNT ) - 1 )
#define TAGS_COUNT ( LENGTH_INT( g_master_config.bar_config.tags ) )
#define CURRENT_SELECTED_TAG_INDEX g_selected_monitor->tags[ g_selected_monitor->current_tag_index ]
#define CURRENT_SELECTED_TAGS g_selected_monitor->tag_set[ g_selected_monitor->selected_tags ]

typedef struct Tag {
        unsigned int previous_tags, current_tags;
        char layout_symbol[ 8 ];
        const struct Layout *layout;
        int inner_horizontal_gap, inner_vertical_gap;
        int outer_horizontal_gap, outer_vertical_gap;
        double master_factor;
        int number_of_clients_in_master_area;
        bool show_bar, top_bar;
        bool enable_gaps;
} Tag_t;

extern void shift_both( const Argument_t *argument );
extern void shift_client( const Argument_t *argument );
extern void shift_view( const Argument_t *argument );
extern void toggle_view_tag( const Argument_t *argument );
extern void toggle_view_tag_bitmask( const Argument_t *argument );
extern void view_tag( const Argument_t *argument );
extern void view_tag_bitmask( const Argument_t *argument );
extern void view_tag_all_monitors( const Argument_t *argument );
extern void view_tag_all_monitors_bitmask( const Argument_t *argument );

extern unsigned int g_scratchpad_tag;

#endif /* TAG_H_ */
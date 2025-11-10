// TODO: Add checks against the monitor size to avoid gaps being larger than the monitor itself. Results in some strange behavior
// TODO: Probably write a macro or something to wrap the massive repeat use of `CURRENT_SELECTED_TAG.X_gap`

#include "gaps.h"

#include <limits.h>
#include <stddef.h>
#include "client.h"
#include "common.h"
#include "monitor.h"
#include "parser.h"
#include "tags.h"
#include "util.h"

void get_gaps( const Monitor_t *monitor, int *outer_horizontal, int *outer_vertical, int *inner_horizontal, int *inner_vertical, unsigned int *number_of_clients ) {

        // Count number of clients on the monitor
        Client_t *client;
        for ( *number_of_clients = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), ( *number_of_clients )++ ) {}

        // Initialize ints
        *inner_horizontal = 0;
        *inner_vertical = 0;
        *outer_horizontal = 0;
        *outer_vertical = 0;

        // Set gap sizes if enabled
        if ( CURRENT_SELECTED_TAG_INDEX.enable_gaps ) {
                *inner_horizontal = CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap;
                *inner_vertical = CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap;

                // Outer gaps disabled when only one client and smart gaps is enabled
                if ( !g_master_config.layout_config.smart_gaps || *number_of_clients != 1 ) {
                        *outer_horizontal = CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap;
                        *outer_vertical = CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap;
                }
        }
}

void increment_gaps( const Argument_t *argument ) {
        set_gaps( CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap + argument->i, CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap + argument->i,
                  CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap + argument->i, CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap + argument->i );
}

void increment_inner_gaps( const Argument_t *argument ) {
        set_gaps( CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap, CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap, CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap + argument->i,
                  CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap + argument->i );
}

void increment_inner_horizontal_gaps( const Argument_t *argument ) {
        set_gaps( CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap, CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap, CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap + argument->i,
                  CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap );
}

void increment_inner_vertical_gaps( const Argument_t *argument ) {
        set_gaps( CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap, CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap, CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap,
                  CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap + argument->i );
}

void increment_number_of_clients_in_master_areas( const Argument_t *argument ) {
        CURRENT_SELECTED_TAG_INDEX.number_of_clients_in_master_area = MAX( CURRENT_SELECTED_TAG_INDEX.number_of_clients_in_master_area + argument->i, 0 );
        arrange( g_selected_monitor );
}

void increment_outer_gaps( const Argument_t *argument ) {
        set_gaps( CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap + argument->i, CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap + argument->i, CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap,
                  CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap );
}

void increment_outer_horizontal_gaps( const Argument_t *argument ) {
        set_gaps( CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap + argument->i, CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap, CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap,
                  CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap );
}

void increment_outer_vertical_gaps( const Argument_t *argument ) {
        set_gaps( CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap, CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap + argument->i, CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap,
                  CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap );
}

void set_gaps( const int outer_horizontal, const int outer_vertical, const int inner_horizontal, const int inner_vertical ) {
        CURRENT_SELECTED_TAG_INDEX.outer_horizontal_gap = clamp_range_int( outer_horizontal, 0, INT_MAX );
        CURRENT_SELECTED_TAG_INDEX.outer_vertical_gap = clamp_range_int( outer_vertical, 0, INT_MAX );
        CURRENT_SELECTED_TAG_INDEX.inner_horizontal_gap = clamp_range_int( inner_horizontal, 0, INT_MAX );
        CURRENT_SELECTED_TAG_INDEX.inner_vertical_gap = clamp_range_int( inner_vertical, 0, INT_MAX );
        arrange( g_selected_monitor );
}

void set_gaps_default( const Argument_t *argument ) {
        set_gaps( g_master_config.layout_config.default_outer_horizontal_gap, g_master_config.layout_config.default_outer_vertical_gap, g_master_config.layout_config.default_inner_horizontal_gap,
                  g_master_config.layout_config.default_inner_vertical_gap );
}

void toggle_gaps( const Argument_t *argument ) {
        CURRENT_SELECTED_TAG_INDEX.enable_gaps = !CURRENT_SELECTED_TAG_INDEX.enable_gaps;
        arrange( NULL );
}

void toggle_smart_gaps( const Argument_t *argument ) {
        g_master_config.layout_config.smart_gaps = !g_master_config.layout_config.smart_gaps;
        arrange( NULL );
}

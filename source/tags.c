#include "tags.h"

#include <stddef.h>
#include <util.h>
#include "bar.h"
#include "client.h"
#include "monitor.h"
#include "xproperty.h"

unsigned int g_scratchpad_tag = 1 << 9;

void shift_both( const Argument_t *argument ) {
        Argument_t shifted = { .ui = CURRENT_SELECTED_TAGS };

        // If i>0 left circular shift, else right circular shift
        if ( argument->i > 0 ) {
                shifted.ui = ( ( shifted.ui << argument->i ) | ( shifted.ui >> ( TAGS_COUNT - argument->i ) ) );
        } else {
                shifted.ui = ( ( shifted.ui >> ( -argument->i ) | shifted.ui << ( TAGS_COUNT + argument->i ) ) );
        }

        client_to_tag_bitmask( &shifted );
        view_tag_bitmask( &shifted );
}

void shift_client( const Argument_t *argument ) {
        Argument_t shifted = { .ui = CURRENT_SELECTED_TAGS };

        // If i>0 left circular shift, else right circular shift
        if ( argument->i > 0 ) {
                shifted.ui = ( ( shifted.ui << argument->i ) | ( shifted.ui >> ( TAGS_COUNT - argument->i ) ) );
        } else {
                shifted.ui = ( shifted.ui >> ( -argument->i ) | shifted.ui << ( TAGS_COUNT + argument->i ) );
        }

        client_to_tag_bitmask( &shifted );
}

void shift_view( const Argument_t *argument ) {
        Argument_t shifted_tags = { .ui = CURRENT_SELECTED_TAGS };

        // If i>0 left circular shift, else right circular shift
        if ( argument->i > 0 ) {
                shifted_tags.ui = ( shifted_tags.ui << argument->i ) | ( shifted_tags.ui >> ( TAGS_COUNT - argument->i ) );
        } else {
                shifted_tags.ui = ( shifted_tags.ui >> ( -argument->i ) | shifted_tags.ui << ( TAGS_COUNT + argument->i ) );
        }

        view_tag_bitmask( &shifted_tags );
}

void toggle_view_tag( const Argument_t *argument ) {
        toggle_view_tag_bitmask( &( Argument_t ) {
                .
                ui = ( 1 << ( argument->ui - 1 ) )
        }
        )
        ;
}

void toggle_view_tag_bitmask( const Argument_t *argument ) {
        const unsigned int new_selected_tags = CURRENT_SELECTED_TAGS ^ ( argument->ui & TAGMASK );

        if ( new_selected_tags ) {
                CURRENT_SELECTED_TAGS = new_selected_tags;

                if ( new_selected_tags == ~0U ) {
                        CURRENT_SELECTED_TAG_INDEX.previous_tags = CURRENT_SELECTED_TAG_INDEX.current_tags;
                        CURRENT_SELECTED_TAG_INDEX.current_tags = 0;
                }

                // Test if the user did not select the same tag
                if ( !( new_selected_tags & 1 << ( CURRENT_SELECTED_TAG_INDEX.current_tags - 1 ) ) ) {
                        CURRENT_SELECTED_TAG_INDEX.previous_tags = CURRENT_SELECTED_TAG_INDEX.current_tags;

                        int i;
                        for ( i = 0; !( new_selected_tags & 1 << i ); i++ ) {}

                        CURRENT_SELECTED_TAG_INDEX.current_tags = i + 1;
                }

                focus( NULL );
                arrange( g_selected_monitor );
        }

        update_current_desktop();
}

void view_tag( const Argument_t *argument ) {
        view_tag_bitmask( &( Argument_t ) {
                .
                ui = ( 1 << ( clamp_range_uint( argument->ui, 1, TAGS_COUNT ) - 1 ) )
        }
        )
        ;
}

void view_tag_bitmask( const Argument_t *argument ) {
        if ( ( argument->ui & TAGMASK ) == CURRENT_SELECTED_TAGS ) return;

        // Toggle sel tagset
        g_selected_monitor->selected_tags ^= 1;

        // Save current tag index
        for ( int i = 0; i < TAGS_COUNT; i++ ) {
                if ( argument->ui & ( 1u << i ) ) {
                        g_selected_monitor->current_tag_index = i;
                        break;
                }
        }

        if ( argument->ui & TAGMASK ) {
                CURRENT_SELECTED_TAGS = argument->ui & TAGMASK;
                CURRENT_SELECTED_TAG_INDEX.previous_tags = CURRENT_SELECTED_TAG_INDEX.current_tags;

                if ( argument->ui == ~0U ) {
                        CURRENT_SELECTED_TAG_INDEX.current_tags = 0;
                } else {
                        int i;
                        for ( i = 0; !( argument->ui & 1 << i ); i++ ) {}
                        CURRENT_SELECTED_TAG_INDEX.current_tags = i + 1;
                }
        } else {
                const unsigned int tmp_tag = CURRENT_SELECTED_TAG_INDEX.previous_tags;
                CURRENT_SELECTED_TAG_INDEX.previous_tags = CURRENT_SELECTED_TAG_INDEX.current_tags;
                CURRENT_SELECTED_TAG_INDEX.current_tags = tmp_tag;
        }

        focus( NULL );
        update_bar_position( g_selected_monitor );
        resize_bar_window( g_selected_monitor );
        arrange( g_selected_monitor );
        update_current_desktop();
}

void view_tag_all_monitors( const Argument_t *argument ) {
        view_tag_all_monitors_bitmask( &( Argument_t ) {
                .
                ui = ( 1 << ( argument->ui - 1 ) )
        }
        )
        ;
}

void view_tag_all_monitors_bitmask( const Argument_t *argument ) {
        for ( Monitor_t *monitor = g_monitors; monitor; monitor = monitor->next_monitor ) {
                monitor->tag_set[ monitor->selected_tags ] = argument->ui;
                arrange( monitor );
        }

        focus( NULL );
}
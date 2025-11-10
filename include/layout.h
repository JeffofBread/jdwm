#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <stdbool.h>
#include "common.h"
#include "monitor.h"

typedef struct Layout {
        const char symbol[ 8 ];
        void ( *arrange )( struct Monitor * );
        const char *name;
} Layout_t;

extern void fibonacci( Monitor_t *monitor, bool spiral );
extern void layout_b_stack( Monitor_t *monitor );
extern void layout_b_stack_horizontal( Monitor_t *monitor );
extern void layout_centered_floating_master( Monitor_t *monitor );
extern void layout_centered_master( Monitor_t *monitor );
extern void layout_deck( Monitor_t *monitor );
extern void layout_dwindle( Monitor_t *monitor );
extern void layout_gapless_grid( Monitor_t *m );
extern void layout_grid( Monitor_t *m );
extern void layout_horizontal_grid( Monitor_t *m );
extern void layout_monocle( Monitor_t *monitor );
extern void layout_narrow_grid( Monitor_t *monitor );
extern void layout_scroll( const Argument_t *argument );
extern void layout_spiral( Monitor_t *monitor );
extern void layout_tile( Monitor_t *monitor );
extern void set_layout( Argument_t *argument );
extern void set_layout_number( const Argument_t *argument );
extern void zoom( const Argument_t *argument );

extern const Layout_t LAYOUTS[ 14 ];
#endif /* LAYOUT_H_ */

#ifndef BAR_H_
#define BAR_H_

#include "common.h"
#include "monitor.h"

extern void draw_bar( Monitor_t *monitor );
extern void draw_bars( void );
extern void resize_bar_window( Monitor_t *monitor );
extern void toggle_bar( const Argument_t *argument );
extern void update_bars( void );
extern void update_bar_position( Monitor_t *monitor );
extern void update_status( void );

extern int g_bar_height;
extern int g_horizontal_bar_padding, g_vertical_bar_padding;
extern char g_status_text[ 128 ];

#endif /* BAR_H_ */

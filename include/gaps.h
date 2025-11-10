#ifndef GAPS_H_
#define GAPS_H_

#include "common.h"
#include "monitor.h"

extern void get_gaps( const Monitor_t *monitor, int *outer_horizontal, int *outer_vertical, int *inner_horizontal, int *inner_vertical, unsigned int *number_of_clients );
extern void increment_gaps( const Argument_t *argument );
extern void increment_inner_gaps( const Argument_t *argument );
extern void increment_inner_horizontal_gaps( const Argument_t *argument );
extern void increment_inner_vertical_gaps( const Argument_t *argument );
extern void increment_number_of_clients_in_master_areas( const Argument_t *argument );
extern void increment_outer_gaps( const Argument_t *argument );
extern void increment_outer_horizontal_gaps( const Argument_t *argument );
extern void increment_outer_vertical_gaps( const Argument_t *argument );
extern void set_gaps( int outer_horizontal, int outer_vertical, int inner_horizontal, int inner_vertical );
extern void set_gaps_default( const Argument_t *argument );
extern void toggle_gaps( const Argument_t *argument );
extern void toggle_smart_gaps( const Argument_t *argument );

#endif /* GAPS_H_ */

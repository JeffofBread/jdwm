#ifndef XPROPERTY_H_
#define XPROPERTY_H_

#include "client.h"

extern void set_client_state( const Client_t *client, long state );
extern void set_client_tag_property( const Client_t *client );
extern void set_current_desktop( void );
extern void set_desktop_names( void );
extern void set_focus( Client_t *client );
extern void set_number_of_desktops( void );
extern void set_viewport( void );
extern void update_client_list( void );
extern void update_current_desktop( void );

#endif /* XPROPERTY_H_ */
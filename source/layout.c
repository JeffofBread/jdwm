// TODO: This whole file is just a mess, it needs major work. Something like a more generic layout function would help majorly.

#include "layout.h"

#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include "bar.h"
#include "client.h"
#include "common.h"
#include "gaps.h"
#include "logger.h"
#include "monitor.h"
#include "parser.h"
#include "tags.h"

// TODO: Find a use for the string layout name or remove it
const Layout_t LAYOUTS[ ] = {
        /* first entry is default */
        { "[]=", layout_tile, "Tile" },
        { "[M]", layout_monocle, "Monocle" },
        { "[@]", layout_spiral, "Spiral" },
        { "[\\]", layout_dwindle, "Dwindle" },
        { "H[]", layout_deck, "Deck" },
        { "TTT", layout_b_stack, "B-Stack" },
        { "===", layout_b_stack_horizontal, "B-Stack Horizontal" },
        { "HHH", layout_grid, "Grid" },
        { "###", layout_narrow_grid, "Narrow Grid" },
        { "---", layout_horizontal_grid, "Horizontal Grid" },
        { ":::", layout_gapless_grid, "Gapless Grid" },
        { "|M|", layout_centered_master, "Centered Master" },
        { ">M>", layout_centered_floating_master, "Centered Floating Master" },

        /* NULL layout function means floating behavior */
        { "><>", NULL, "Floating" },
};

void fibonacci( Monitor_t *monitor, const bool spiral ) {
        unsigned int number_of_clients;
        int outer_horizontal_gap, outer_vertical_gap, inner_horizontal_gap, inner_vertical_gap;
        get_gaps( monitor, &outer_horizontal_gap, &outer_vertical_gap, &inner_horizontal_gap, &inner_vertical_gap, &number_of_clients );

        if ( number_of_clients == 0 ) return;

        int new_x = monitor->window_x + outer_vertical_gap;
        int new_y = monitor->window_y + outer_horizontal_gap;
        int new_width = monitor->window_width - 2 * outer_vertical_gap;
        int new_height = monitor->window_height - 2 * outer_horizontal_gap;

        Client_t *client;
        unsigned int i, running = 1;
        int height_remainder = 0, width_remainder = 0;
        for ( i = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ) ) {
                if ( running ) {
                        if ( ( i % 2 && ( new_height - inner_horizontal_gap ) / 2 <= ( g_bar_height + 2 * client->border_width ) ) || (
                                     !( i % 2 ) && ( new_width - inner_vertical_gap ) / 2 <= ( g_bar_height + 2 * client->border_width ) ) ) {
                                running = 0;
                        }

                        if ( running && i < number_of_clients - 1 ) {
                                int tmp;
                                if ( i % 2 ) {
                                        tmp = ( new_height - inner_horizontal_gap ) / 2;
                                        height_remainder = new_height - 2 * tmp - inner_horizontal_gap;
                                        new_height = tmp;
                                } else {
                                        tmp = ( new_width - inner_vertical_gap ) / 2;
                                        width_remainder = new_width - 2 * tmp - inner_vertical_gap;
                                        new_width = tmp;
                                }

                                if ( ( i % 4 ) == 2 && spiral ) {
                                        new_x += new_width + inner_vertical_gap;
                                } else if ( ( i % 4 ) == 3 && spiral ) {
                                        new_y += new_height + inner_horizontal_gap;
                                }
                        }

                        if ( ( i % 4 ) == 0 ) {
                                if ( !spiral ) {
                                        new_y += new_height + inner_horizontal_gap;
                                        new_height += height_remainder;
                                } else {
                                        new_height -= height_remainder;
                                        new_y -= new_height + inner_horizontal_gap;
                                }
                        } else if ( ( i % 4 ) == 1 ) {
                                new_x += new_width + inner_vertical_gap;
                                new_width += width_remainder;
                        } else if ( ( i % 4 ) == 2 ) {
                                new_y += new_height + inner_horizontal_gap;
                                new_height += height_remainder;
                                if ( i < number_of_clients - 1 ) {
                                        new_width += width_remainder;
                                }
                        } else if ( ( i % 4 ) == 3 ) {
                                if ( !spiral ) {
                                        new_x += new_width + inner_vertical_gap;
                                        new_width -= width_remainder;
                                } else {
                                        new_width -= width_remainder;
                                        new_x -= new_width + inner_vertical_gap;
                                        new_height += height_remainder;
                                }
                        }

                        if ( i == 0 ) {
                                if ( number_of_clients != 1 ) {
                                        new_width = (int) ( ( monitor->window_width - inner_vertical_gap - 2 * outer_vertical_gap ) - (
                                                                    monitor->window_width - inner_vertical_gap - 2 * outer_vertical_gap ) * (
                                                                    1 - monitor->tags[ monitor->current_tag_index ].master_factor ) );
                                        width_remainder = 0;
                                }

                                new_y = monitor->window_y + outer_horizontal_gap;
                        } else if ( i == 1 ) {
                                new_width = monitor->window_width - new_width - inner_vertical_gap - 2 * outer_vertical_gap;
                        }

                        i++;
                }

                resize( client, new_x, new_y, new_width - ( 2 * client->border_width ), new_height - ( 2 * client->border_width ), False );
        }
}

void layout_b_stack( Monitor_t *monitor ) {
        unsigned int i, numberofclients;
        int outerhorizontalgap, outerverticalgap, innerhorizontalgap, innerverticalgap;
        int masterareax = 0, masterareay = 0, masterareaheight = 0, masterareawidth = 0;
        int stackareax = 0, stackareay = 0, stackareaheight = 0, stackareawidth = 0;
        double mastorfactors, sfacts;
        int masterarearemainder, stackarearemainder;
        Client_t *client;

        get_gaps( monitor, &outerhorizontalgap, &outerverticalgap, &innerhorizontalgap, &innerverticalgap, &numberofclients );
        if ( numberofclients == 0 ) return;

        stackareax = masterareax = monitor->window_x + outerverticalgap;
        stackareay = masterareay = monitor->window_y + outerhorizontalgap;
        stackareaheight = masterareaheight = monitor->window_height - 2 * outerhorizontalgap;
        masterareawidth = monitor->window_width - 2 * outerverticalgap - innerverticalgap * (
                                  MIN( numberofclients, monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) - 1 );
        stackareawidth = monitor->window_width - 2 * outerverticalgap - innerverticalgap * ( numberofclients - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area - 1 );

        if ( monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area && monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                stackareaheight = ( masterareaheight - innerhorizontalgap ) * ( 1 - monitor->tags[ monitor->current_tag_index ].master_factor );
                masterareaheight = masterareaheight - innerhorizontalgap - stackareaheight;
                stackareax = masterareax;
                stackareay = masterareay + masterareaheight + innerhorizontalgap;
        }

        get_facts( monitor, masterareawidth, stackareawidth, &mastorfactors, &sfacts, &masterarearemainder, &stackarearemainder );

        for ( i = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), i++ ) {
                if ( i < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                        resize( client, masterareax, masterareay, ( masterareawidth / mastorfactors ) + ( i < masterarearemainder ? 1 : 0 ) - ( 2 * client->border_width ),
                                masterareaheight - ( 2 * client->border_width ), 0 );
                        masterareax += WIDTH( client ) + innerverticalgap;
                } else {
                        resize( client, stackareax, stackareay,
                                ( stackareawidth / sfacts ) + ( ( i - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) < stackarearemainder ? 1 : 0 ) - (
                                        2 * client->border_width ), stackareaheight - ( 2 * client->border_width ), 0 );
                        stackareax += WIDTH( client ) + innerverticalgap;
                }
        }
}

void layout_b_stack_horizontal( Monitor_t *monitor ) {
        unsigned int i, numberofclients;
        int outerhorizontalgap, outerverticalgap, innerhorizontalgap, innerverticalgap;
        int masterareax = 0, masterareay = 0, masterareaheight = 0, masterareawidth = 0;
        int stackareax = 0, stackareay = 0, stackareaheight = 0, stackareawidth = 0;
        double mastorfactors, sfacts;
        int masterarearemainder, stackarearemainder;
        Client_t *client;

        get_gaps( monitor, &outerhorizontalgap, &outerverticalgap, &innerhorizontalgap, &innerverticalgap, &numberofclients );
        if ( numberofclients == 0 ) return;

        stackareax = masterareax = monitor->window_x + outerverticalgap;
        stackareay = masterareay = monitor->window_y + outerhorizontalgap;
        masterareaheight = monitor->window_height - 2 * outerhorizontalgap;
        stackareaheight = monitor->window_height - 2 * outerhorizontalgap - innerhorizontalgap * ( numberofclients - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area - 1 );
        masterareawidth = monitor->window_width - 2 * outerverticalgap - innerverticalgap * (
                                  MIN( numberofclients, monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) - 1 );
        stackareawidth = monitor->window_width - 2 * outerverticalgap;

        if ( monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area && numberofclients > monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                stackareaheight = ( masterareaheight - innerhorizontalgap ) * ( 1 - monitor->tags[ monitor->current_tag_index ].master_factor );
                masterareaheight = masterareaheight - innerhorizontalgap - stackareaheight;
                stackareay = masterareay + masterareaheight + innerhorizontalgap;
                stackareaheight = monitor->window_height - masterareaheight - 2 * outerhorizontalgap - innerhorizontalgap * (
                                          numberofclients - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area );
        }

        get_facts( monitor, masterareawidth, stackareaheight, &mastorfactors, &sfacts, &masterarearemainder, &stackarearemainder );

        for ( i = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), i++ ) {
                if ( i < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                        resize( client, masterareax, masterareay, ( masterareawidth / mastorfactors ) + ( i < masterarearemainder ? 1 : 0 ) - ( 2 * client->border_width ),
                                masterareaheight - ( 2 * client->border_width ), 0 );
                        masterareax += WIDTH( client ) + innerverticalgap;
                } else {
                        resize( client, stackareax, stackareay, stackareawidth - ( 2 * client->border_width ),
                                ( stackareaheight / sfacts ) + ( ( i - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) < stackarearemainder ? 1 : 0 ) - (
                                        2 * client->border_width ), 0 );
                        stackareay += HEIGHT( client ) + innerhorizontalgap;
                }
        }
}

void layout_centered_floating_master( Monitor_t *monitor ) {
        unsigned int i, n;
        double mastorfactors, sfacts;
        double mivf = 1.0; // master inner vertical gap factor
        int oh, ov, ih, iv, mrest, srest;
        int mx = 0, my = 0, mh = 0, mw = 0;
        int sx = 0, sy = 0, sh = 0, sw = 0;
        Client_t *c;

        get_gaps( monitor, &oh, &ov, &ih, &iv, &n );
        if ( n == 0 ) return;

        sx = mx = monitor->window_x + ov;
        sy = my = monitor->window_y + oh;
        sh = mh = monitor->window_height - 2 * oh;
        mw = monitor->window_width - 2 * ov - iv * ( n - 1 );
        sw = monitor->window_width - 2 * ov - iv * ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area - 1 );

        if ( monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area && n > monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                mivf = 0.8;
                /* go mastorfactor box in the center if more than numbermastorfactor clients */
                if ( monitor->window_width > monitor->window_height ) {
                        mw = monitor->window_width * monitor->tags[ monitor->current_tag_index ].master_factor - iv * mivf * (
                                     MIN( n, monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) - 1 );
                        mh = monitor->window_height * 0.9;
                } else {
                        mw = monitor->window_width * 0.9 - iv * mivf * ( MIN( n, monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) - 1 );
                        mh = monitor->window_height * monitor->tags[ monitor->current_tag_index ].master_factor;
                }

                mx = monitor->window_x + ( monitor->window_width - mw ) / 2;
                my = monitor->window_y + ( monitor->window_height - mh - 2 * oh ) / 2;

                sx = monitor->window_x + ov;
                sy = monitor->window_y + oh;
                sh = monitor->window_height - 2 * oh;
        }

        get_facts( monitor, mw, sw, &mastorfactors, &sfacts, &mrest, &srest );

        for ( i = 0, c = next_tiled( monitor->client_list ); c; c = next_tiled( c->next_client ), i++ )
                if ( i < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                        /* numbermastorfactor clients are stacked horizontally, in the center of the screen */
                        resize( c, mx, my, ( mw / mastorfactors ) + ( i < mrest ? 1 : 0 ) - ( 2 * c->border_width ), mh - ( 2 * c->border_width ), 0 );
                        mx += WIDTH( c ) + iv * mivf;
                } else {
                        /* stack clients are stacked horizontally */
                        resize( c, sx, sy, ( sw / sfacts ) + ( ( i - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) < srest ? 1 : 0 ) - ( 2 * c->border_width ),
                                sh - ( 2 * c->border_width ), 0 );
                        sx += WIDTH( c ) + iv;
                }
}

void layout_centered_master( Monitor_t *monitor ) {
        unsigned int i, n;
        int oh, ov, ih, iv;
        int mx = 0, my = 0, mh = 0, mw = 0;
        int lx = 0, ly = 0, lw = 0, lh = 0;
        int rx = 0, ry = 0, rw = 0, rh = 0;
        double mastorfactors = 0, lfacts = 0, rfacts = 0;
        int mtotal = 0, ltotal = 0, rtotal = 0;
        int mrest = 0, lrest = 0, rrest = 0;
        Client_t *client;

        get_gaps( monitor, &oh, &ov, &ih, &iv, &n );
        if ( n == 0 ) return;

        /* initialize areas */
        mx = monitor->window_x + ov;
        my = monitor->window_y + oh;
        mh = monitor->window_height - 2 * oh - ih * ( ( !monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area
                                                                ? n
                                                                : MIN( n, monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) ) - 1 );
        mw = monitor->window_width - 2 * ov;
        lh = monitor->window_height - 2 * oh - ih * ( ( ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) / 2 ) - 1 );
        rh = monitor->window_height - 2 * oh - ih * ( ( ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) / 2 ) - (
                                                              ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) % 2 ? 0 : 1 ) );

        if ( monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area && n > monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                /* go mastorfactor box in the center if more than numbermastorfactor clients */
                if ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area > 1 ) {
                        /* ||<-S->|<---M--->|<-S->|| */
                        mw = ( monitor->window_width - 2 * ov - 2 * iv ) * monitor->tags[ monitor->current_tag_index ].master_factor;
                        lw = ( monitor->window_width - mw - 2 * ov - 2 * iv ) / 2;
                        rw = ( monitor->window_width - mw - 2 * ov - 2 * iv ) - lw;
                        mx += lw + iv;
                } else {
                        /* ||<---M--->|<-S->|| */
                        mw = ( mw - iv ) * monitor->tags[ monitor->current_tag_index ].master_factor;
                        lw = 0;
                        rw = monitor->window_width - mw - iv - 2 * ov;
                }
                lx = monitor->window_x + ov;
                ly = monitor->window_y + oh;
                rx = mx + mw + iv;
                ry = monitor->window_y + oh;
        }

        /* calculate facts */
        for ( n = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), n++ ) {
                if ( !monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area || n < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area )
                        mastorfactors += 1;
                else if ( ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) % 2 ) lfacts += 1; // total factor of left hand stack area
                else rfacts += 1;                                                                                                 // total factor of right hand stack area
        }

        for ( n = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), n++ )
                if ( !monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area || n < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area )
                        mtotal += mh / mastorfactors;
                else if ( ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) % 2 ) ltotal += lh / lfacts;
                else rtotal += rh / rfacts;

        mrest = mh - mtotal;
        lrest = lh - ltotal;
        rrest = rh - rtotal;

        for ( i = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), i++ ) {
                if ( !monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area || i < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                        /* numbermastorfactor clients are stacked vertically, in the center of the screen */
                        resize( client, mx, my, mw - ( 2 * client->border_width ), ( mh / mastorfactors ) + ( i < mrest ? 1 : 0 ) - ( 2 * client->border_width ), 0 );
                        my += HEIGHT( client ) + ih;
                } else {
                        /* stack clients are stacked vertically */
                        if ( ( i - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) % 2 ) {
                                resize( client, lx, ly, lw - ( 2 * client->border_width ),
                                        ( lh / lfacts ) + ( ( i - 2 * monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) < 2 * lrest ? 1 : 0 ) - (
                                                2 * client->border_width ), 0 );
                                ly += HEIGHT( client ) + ih;
                        } else {
                                resize( client, rx, ry, rw - ( 2 * client->border_width ),
                                        ( rh / rfacts ) + ( ( i - 2 * monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) < 2 * rrest ? 1 : 0 ) - (
                                                2 * client->border_width ), 0 );
                                ry += HEIGHT( client ) + ih;
                        }
                }
        }
}

void layout_deck( Monitor_t *monitor ) {
        unsigned int i, n;
        int oh, ov, ih, iv;
        int mx = 0, my = 0, mh = 0, mw = 0;
        int sx = 0, sy = 0, sh = 0, sw = 0;
        double mastorfactors, sfacts;
        int mrest, srest;
        Client_t *client;

        get_gaps( monitor, &oh, &ov, &ih, &iv, &n );
        if ( n == 0 ) return;

        sx = mx = monitor->window_x + ov;
        sy = my = monitor->window_y + oh;
        sh = mh = monitor->window_height - 2 * oh - ih * ( MIN( n, monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) - 1 );
        sw = mw = monitor->window_width - 2 * ov;

        if ( monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area && n > monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                sw = ( mw - iv ) * ( 1 - monitor->tags[ monitor->current_tag_index ].master_factor );
                mw = mw - iv - sw;
                sx = mx + mw + iv;
                sh = monitor->window_height - 2 * oh;
        }

        get_facts( monitor, mh, sh, &mastorfactors, &sfacts, &mrest, &srest );

        if ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area > 0 ) /* override layout symbol */
                snprintf( monitor->tags[ monitor->current_tag_index ].layout_symbol, sizeof( monitor->tags[ monitor->current_tag_index ].layout_symbol ), "D %d",
                          n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area );

        for ( i = 0, client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), i++ )
                if ( i < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                        resize( client, mx, my, mw - ( 2 * client->border_width ), ( mh / mastorfactors ) + ( i < mrest ? 1 : 0 ) - ( 2 * client->border_width ), 0 );
                        my += HEIGHT( client ) + ih;
                } else {
                        resize( client, sx, sy, sw - ( 2 * client->border_width ), sh - ( 2 * client->border_width ), 0 );
                }
}

void layout_dwindle( Monitor_t *monitor ) {
        fibonacci( monitor, false );
}

void layout_gapless_grid( Monitor_t *m ) {
        unsigned int i, n;
        int x, y, cols, rows, ch, cw, cn, rn, rrest, crest; // counters
        int oh, ov, ih, iv;
        Client_t *c;

        get_gaps( m, &oh, &ov, &ih, &iv, &n );
        if ( n == 0 ) return;

        /* grid dimensions */
        for ( cols = 0; cols <= n / 2; cols++ ) if ( cols * cols >= n ) break;
        if ( n == 5 ) /* set layout against the general calculation: not 1:2:2, but 2:3 */
                cols = 2;
        rows = n / cols;
        cn = rn = 0; // reset column no, row no, client count

        ch = ( m->window_height - 2 * oh - ih * ( rows - 1 ) ) / rows;
        cw = ( m->window_width - 2 * ov - iv * ( cols - 1 ) ) / cols;
        rrest = ( m->window_height - 2 * oh - ih * ( rows - 1 ) ) - ch * rows;
        crest = ( m->window_width - 2 * ov - iv * ( cols - 1 ) ) - cw * cols;
        x = m->window_x + ov;
        y = m->window_y + oh;

        for ( i = 0, c = next_tiled( m->client_list ); c; i++, c = next_tiled( c->next_client ) ) {
                if ( i / rows + 1 > cols - n % cols ) {
                        rows = n / cols + 1;
                        ch = ( m->window_height - 2 * oh - ih * ( rows - 1 ) ) / rows;
                        rrest = ( m->window_height - 2 * oh - ih * ( rows - 1 ) ) - ch * rows;
                }
                resize( c, x, y + rn * ( ch + ih ) + MIN( rn, rrest ), cw + ( cn < crest ? 1 : 0 ) - 2 * c->border_width, ch + ( rn < rrest ? 1 : 0 ) - 2 * c->border_width, 0 );
                rn++;
                if ( rn >= rows ) {
                        rn = 0;
                        x += cw + ih + ( cn < crest ? 1 : 0 );
                        cn++;
                }
        }
}

void layout_grid( Monitor_t *m ) {
        unsigned int i, n;
        int cx, cy, cw, ch, cc, cr, chrest, cwrest, cols, rows;
        int oh, ov, ih, iv;
        Client_t *c;

        get_gaps( m, &oh, &ov, &ih, &iv, &n );

        /* grid dimensions */
        for ( rows = 0; rows <= n / 2; rows++ ) if ( rows * rows >= n ) break;
        cols = ( rows && ( rows - 1 ) * rows >= n ) ? rows - 1 : rows;

        /* window geoms (cell height/width) */
        ch = ( m->window_height - 2 * oh - ih * ( rows - 1 ) ) / ( rows ? rows : 1 );
        cw = ( m->window_width - 2 * ov - iv * ( cols - 1 ) ) / ( cols ? cols : 1 );
        chrest = ( m->window_height - 2 * oh - ih * ( rows - 1 ) ) - ch * rows;
        cwrest = ( m->window_width - 2 * ov - iv * ( cols - 1 ) ) - cw * cols;
        for ( i = 0, c = next_tiled( m->client_list ); c; c = next_tiled( c->next_client ), i++ ) {
                cc = i / rows;
                cr = i % rows;
                cx = m->window_x + ov + cc * ( cw + iv ) + MIN( cc, cwrest );
                cy = m->window_y + oh + cr * ( ch + ih ) + MIN( cr, chrest );
                resize( c, cx, cy, cw + ( cc < cwrest ? 1 : 0 ) - 2 * c->border_width, ch + ( cr < chrest ? 1 : 0 ) - 2 * c->border_width, False );
        }
}

void layout_horizontal_grid( Monitor_t *m ) {
        Client_t *c;
        unsigned int n, i;
        int oh, ov, ih, iv;
        int mx = 0, my = 0, mh = 0, mw = 0;
        int sx = 0, sy = 0, sh = 0, sw = 0;
        int ntop, nbottom = 1;
        float mastorfactors, sfacts;
        int mrest, srest;

        /* Count windows */
        get_gaps( m, &oh, &ov, &ih, &iv, &n );
        if ( n == 0 ) return;

        if ( n <= 2 ) ntop = n;
        else {
                ntop = n / 2;
                nbottom = n - ntop;
        }
        sx = mx = m->window_x + ov;
        sy = my = m->window_y + oh;
        sh = mh = m->window_height - 2 * oh;
        sw = mw = m->window_width - 2 * ov;

        if ( n > ntop ) {
                sh = ( mh - ih ) / 2;
                mh = mh - ih - sh;
                sy = my + mh + ih;
                mw = m->window_width - 2 * ov - iv * ( ntop - 1 );
                sw = m->window_width - 2 * ov - iv * ( nbottom - 1 );
        }

        mastorfactors = ntop;
        sfacts = nbottom;
        mrest = mw - ( mw / ntop ) * ntop;
        srest = sw - ( sw / nbottom ) * nbottom;

        for ( i = 0, c = next_tiled( m->client_list ); c; c = next_tiled( c->next_client ), i++ )
                if ( i < ntop ) {
                        resize( c, mx, my, ( mw / mastorfactors ) + ( i < mrest ? 1 : 0 ) - ( 2 * c->border_width ), mh - ( 2 * c->border_width ), 0 );
                        mx += WIDTH( c ) + iv;
                } else {
                        resize( c, sx, sy, ( sw / sfacts ) + ( ( i - ntop ) < srest ? 1 : 0 ) - ( 2 * c->border_width ), sh - ( 2 * c->border_width ), 0 );
                        sx += WIDTH( c ) + iv;
                }
}

void layout_monocle( Monitor_t *monitor ) {
        unsigned int n;
        int oh, ov, ih, iv;

        get_gaps( monitor, &oh, &ov, &ih, &iv, &n );

        /* override layout symbol */
        if ( n > 0 ) {
                snprintf( monitor->tags[ monitor->current_tag_index ].layout_symbol, sizeof( monitor->tags[ monitor->current_tag_index ].layout_symbol ), "[%d]", n );
        }

        for ( Client_t *client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ) ) {
                resize( client, monitor->window_x + ov, monitor->window_y + oh, monitor->window_width - 2 * client->border_width - 2 * ov, monitor->window_height - 2 * client->border_width - 2 * oh,
                        0 );
        }
}

void layout_narrow_grid( Monitor_t *monitor ) {
        unsigned int n;
        int ri = 0, ci = 0;                  /* counters */
        int oh, ov, ih, iv;                  /* vanitygap settings */
        unsigned int cx, cy, cw, ch;         /* client geometry */
        unsigned int uw = 0, uh = 0, uc = 0; /* utilization trackers */
        unsigned int cols, rows = monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area + 1;

        /* count clients */
        get_gaps( monitor, &oh, &ov, &ih, &iv, &n );

        /* nothing to do here */
        if ( n == 0 ) return;

        /* force 2 clients to always split vertically */
        if ( g_master_config.layout_config.force_vertical_split && n == 2 ) rows = 1;

        /* never allow empty rows */
        if ( n < rows ) rows = n;

        /* define first row */
        cols = n / rows;
        uc = cols;
        cy = monitor->window_y + oh;
        ch = ( monitor->window_height - 2 * oh - ih * ( rows - 1 ) ) / rows;
        uh = ch;

        for ( Client_t *client = next_tiled( monitor->client_list ); client; client = next_tiled( client->next_client ), ci++ ) {
                if ( ci == cols ) {
                        uw = 0;
                        ci = 0;
                        ri++;

                        /* next row */
                        cols = ( n - uc ) / ( rows - ri );
                        uc += cols;
                        cy = monitor->window_y + oh + uh + ih;
                        uh += ch + ih;
                }

                cx = monitor->window_x + ov + uw;
                cw = ( monitor->window_width - 2 * ov - uw ) / ( cols - ci );
                uw += cw + iv;

                resize( client, cx, cy, cw - ( 2 * client->border_width ), ch - ( 2 * client->border_width ), 0 );
        }
}

void layout_scroll( const Argument_t *argument ) {
        if ( !argument || !argument->i ) return;

        // Save length so we don't need to re-compute it over and over
        const int layouts_count = LENGTH( LAYOUTS );

        // Find the current index from the pointer
        int layout_index = (int) ( CURRENT_SELECTED_TAG_INDEX.layout - LAYOUTS );

        // If we don't find the layout or if it's out of bounds, error and return
        if ( layout_index >= layouts_count || layout_index < 0 ) {
                log_error( "Unable to find current layout's index. Computed index: %d", layout_index );
                return;
        }

        // Add our difference/scroll
        layout_index += (int) argument->i;

        // Check its in bounds again
        if ( layout_index >= layouts_count ) {
                layout_index = 0;
        } else if ( layout_index < 0 ) {
                layout_index = layouts_count - 1;
        }

        // Save and set new layout
        CURRENT_SELECTED_TAG_INDEX.layout = &LAYOUTS[ layout_index ];
        set_layout( &(Argument_t){ .v = CURRENT_SELECTED_TAG_INDEX.layout } );
}

void layout_spiral( Monitor_t *monitor ) {
        fibonacci( monitor, true );
}

void layout_tile( Monitor_t *monitor ) {
        unsigned int i, n;
        int oh, ov, ih, iv;
        int mx = 0, my = 0, mh = 0, mw = 0;
        int sx = 0, sy = 0, sh = 0, sw = 0;
        double mastorfactors, sfacts;
        int mrest, srest;
        Client_t *c;

        get_gaps( monitor, &oh, &ov, &ih, &iv, &n );
        if ( n == 0 ) return;

        sx = mx = monitor->window_x + ov;
        sy = my = monitor->window_y + oh;
        mh = monitor->window_height - 2 * oh - ih * ( MIN( n, monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) - 1 );
        sh = monitor->window_height - 2 * oh - ih * ( n - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area - 1 );
        sw = mw = monitor->window_width - 2 * ov;

        if ( monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area && n > monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                sw = ( mw - iv ) * ( 1 - monitor->tags[ monitor->current_tag_index ].master_factor );
                mw = mw - iv - sw;
                sx = mx + mw + iv;
        }

        get_facts( monitor, mh, sh, &mastorfactors, &sfacts, &mrest, &srest );

        for ( i = 0, c = next_tiled( monitor->client_list ); c; c = next_tiled( c->next_client ), i++ )
                if ( i < monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) {
                        resize( c, mx, my, mw - ( 2 * c->border_width ), ( mh / mastorfactors ) + ( i < mrest ? 1 : 0 ) - ( 2 * c->border_width ), 0 );
                        my += HEIGHT( c ) + ih;
                } else {
                        resize( c, sx, sy, sw - ( 2 * c->border_width ),
                                ( sh / sfacts ) + ( ( i - monitor->tags[ monitor->current_tag_index ].number_of_clients_in_master_area ) < srest ? 1 : 0 ) - ( 2 * c->border_width ), 0 );
                        sy += HEIGHT( c ) + ih;
                }
}

void set_layout( Argument_t *argument ) {
        if ( !argument || !argument->v ) return;

        CURRENT_SELECTED_TAG_INDEX.layout = (const Layout_t *) argument->v;

        memcpy( CURRENT_SELECTED_TAG_INDEX.layout_symbol, CURRENT_SELECTED_TAG_INDEX.layout->symbol, sizeof( CURRENT_SELECTED_TAG_INDEX.layout->symbol ) );

        if ( g_selected_monitor->selected_client ) {
                arrange( g_selected_monitor );
        } else {
                draw_bar( g_selected_monitor );
        }
}

void set_layout_safe( Argument_t *argument ) {
        const Layout_t *layout_pointer = (Layout_t *) argument->v;
        if ( layout_pointer == 0 ) set_layout( argument );
        for ( int i = 0; i < LENGTH( LAYOUTS ); i++ ) {
                if ( layout_pointer == &LAYOUTS[ i ] ) set_layout( argument );
        }
}

void set_layout_number( const Argument_t *argument ) {
        set_layout( &(Argument_t){ .v = &LAYOUTS[ argument->ui ] } );
}

void zoom( const Argument_t *argument ) {
        Client_t *client = g_selected_monitor->selected_client;
        if ( !CURRENT_SELECTED_TAG_INDEX.layout->arrange || !client || client->is_floating ) return;
        if ( client == next_tiled( g_selected_monitor->client_list ) && !( ( client = next_tiled( client->next_client ) ) ) ) return;
        pop( client );
}

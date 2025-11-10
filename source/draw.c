#include "draw.h"

#include <Imlib2.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/Xft/XftCompat.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/render.h>
#include <fontconfig/fontconfig.h>
#include "common.h"
#include "logger.h"
#include "util.h"

Internal_Drawable_t *g_drawable;

int drawable_color_create( Internal_Drawable_t *drawable, XftColor *destination, const char *color_hex ) {
        if ( !drawable || !destination || !color_hex ) return -1;

        if ( !XftColorAllocName( drawable->display, DefaultVisual( drawable->display, drawable->screen ), DefaultColormap( drawable->display, drawable->screen ), color_hex, destination ) ) {
                log_error( "Could not allocate color '%s'", color_hex );
                return -1;
        }

        return 0;
}

Internal_Drawable_t *drawable_create( Display *display, const int screen, const Window root_window, const unsigned int width, const unsigned int height ) {
        Internal_Drawable_t *drawable = ecalloc( 1, sizeof( Internal_Drawable_t ) );

        drawable->w = width;
        drawable->h = height;
        drawable->display = display;
        drawable->screen = screen;
        drawable->root_window = root_window;
        drawable->drawable = XCreatePixmap( display, root_window, width, height, DefaultDepth( display, screen ) );
        drawable->picture = XRenderCreatePicture( display, drawable->drawable, XRenderFindVisualFormat( display, DefaultVisual( display, screen ) ), 0, NULL );
        drawable->graphics_context = XCreateGC( display, root_window, 0, NULL );

        XSetLineAttributes( display, drawable->graphics_context, 1, LineSolid, CapButt, JoinMiter );

        return drawable;
}

Cursor *drawable_cursor_create( Internal_Drawable_t *drawable, const int shape ) {
        Cursor *cursor = ecalloc( 1, sizeof( Cursor ) );
        if ( !drawable || !cursor ) return NULL;
        *cursor = XCreateFontCursor( drawable->display, shape );
        return cursor;
}

void drawable_cursor_free( const Internal_Drawable_t *drawable, Cursor *cursor ) {
        if ( !cursor ) return;
        XFreeCursor( drawable->display, *cursor );
        free( cursor );
}

Internal_Font_t *drawable_font_create( Internal_Drawable_t *drawable, char fonts[ ] ) {
        Internal_Font_t *tmp, *head = NULL;

        if ( !drawable || !fonts ) return NULL;

        if ( ( tmp = xfont_create( drawable, fonts, NULL ) ) ) {
                tmp->next = head;
                head = tmp;
        }

        return drawable->fonts = head;
}

void drawable_fontset_free( Internal_Font_t *fontset ) {
        if ( fontset ) {
                drawable_fontset_free( fontset->next );
                xfont_free( fontset );
        }
}

unsigned int drawable_fontset_getwidth( Internal_Drawable_t *drawable, const char *text ) {
        if ( !drawable || !drawable->fonts || !text ) return 0;
        return draw_text( drawable, 0, 0, 0, 0, 0, text, 0 );
}

void drawable_font_get_extents( Internal_Font_t *font, const char *text, const unsigned int length, unsigned int *width, unsigned int *height ) {
        if ( !font || !text ) return;

        XGlyphInfo glyph_info;
        XftTextExtentsUtf8( font->display, font->xfont, (XftChar8 *) text, (int) length, &glyph_info );

        // `xOff` is the "Horizontal margin to the next Glyph."
        if ( width ) *width = glyph_info.xOff;

        if ( height ) *height = font->h;
}

void drawable_free( Internal_Drawable_t *drawable ) {
        XRenderFreePicture( drawable->display, drawable->picture );
        XFreePixmap( drawable->display, drawable->drawable );
        XFreeGC( drawable->display, drawable->graphics_context );
        drawable_fontset_free( drawable->fonts );
        free( drawable );
}

void draw_map( Internal_Drawable_t *drawable, const Window window, const int x, const int y, const unsigned int width, const unsigned int height ) {
        if ( !drawable ) return;
        XCopyArea( drawable->display, drawable->drawable, window, drawable->graphics_context, x, y, width, height, x, y );
        XFlush( drawable->display ); // https://github.com/mihirlad55/dwm-ipc/issues/22
}

void draw_picture( Internal_Drawable_t *drawable, const int x, const int y, const unsigned int width, const unsigned int height, const Picture picture ) {
        if ( !drawable ) return;
        XRenderComposite( drawable->display, PictOpOver, picture, None, drawable->picture, 0, 0, 0, 0, x, y, width, height );
}

// TODO: Rename?
Picture drawable_picture_create_resized( const Internal_Drawable_t *drawable, char *source, const unsigned int source_width, const unsigned int source_height, const unsigned int destination_width,
                                         const unsigned int destination_height ) {

        Pixmap pixmap;
        Picture picture;
        GC graphics_context;

        if ( source_width <= ( destination_width << 1u ) && source_height <= ( destination_height << 1u ) ) {

                XImage img = {
                        .width = (int) source_width,
                        .height = (int) source_height,
                        .xoffset = 0,
                        .format = ZPixmap,
                        .data = source,
                        .byte_order = ImageByteOrder( drawable->display ),
                        .bitmap_unit = BitmapUnit( drawable->display ),
                        .bitmap_bit_order = BitmapBitOrder( drawable->display ),
                        .bitmap_pad = 32,
                        .depth = 32,
                        .bytes_per_line = 0,
                        .bits_per_pixel = 32,
                        .red_mask = 0,
                        .green_mask = 0,
                        .blue_mask = 0
                };

                XInitImage( &img );

                pixmap = XCreatePixmap( drawable->display, drawable->root_window, source_width, source_height, 32 );
                graphics_context = XCreateGC( drawable->display, pixmap, 0, NULL );

                XPutImage( drawable->display, pixmap, graphics_context, &img, 0, 0, 0, 0, source_width, source_height );
                XFreeGC( drawable->display, graphics_context );

                picture = XRenderCreatePicture( drawable->display, pixmap, XRenderFindStandardFormat( drawable->display, PictStandardARGB32 ), 0, NULL );

                XFreePixmap( drawable->display, pixmap );

                XRenderSetPictureFilter( drawable->display, picture, FilterBilinear, NULL, 0 );

                XTransform transform;
                transform.matrix[ 0 ][ 0 ] = (int) ( ( source_width << 16u ) / destination_width );
                transform.matrix[ 0 ][ 1 ] = 0;
                transform.matrix[ 0 ][ 2 ] = 0;
                transform.matrix[ 1 ][ 0 ] = 0;
                transform.matrix[ 1 ][ 1 ] = (int) ( ( source_height << 16u ) / destination_height );
                transform.matrix[ 1 ][ 2 ] = 0;
                transform.matrix[ 2 ][ 0 ] = 0;
                transform.matrix[ 2 ][ 1 ] = 0;
                transform.matrix[ 2 ][ 2 ] = 1 << 16;

                XRenderSetPictureTransform( drawable->display, picture, &transform );
        } else {
                Imlib_Image origin = imlib_create_image_using_data( (int) source_width, (int) source_height, (DATA32 *) source );
                if ( !origin ) return None;

                imlib_context_set_image( origin );
                imlib_image_set_has_alpha( 1 );

                Imlib_Image scaled = imlib_create_cropped_scaled_image( 0, 0, (int) source_width, (int) source_height, (int) destination_width, (int) destination_height );
                imlib_free_image_and_decache();
                if ( !scaled ) return None;

                imlib_context_set_image( scaled );
                imlib_image_set_has_alpha( 1 );

                XImage img = {
                        .width = (int) destination_width,
                        .height = (int) destination_height,
                        .xoffset = 0,
                        .format = ZPixmap,
                        .data = (char *) imlib_image_get_data_for_reading_only(),
                        .byte_order = ImageByteOrder( drawable->display ),
                        .bitmap_unit = BitmapUnit( drawable->display ),
                        .bitmap_bit_order = BitmapBitOrder( drawable->display ),
                        .bitmap_pad = 32,
                        .depth = 32,
                        .bytes_per_line = 0,
                        .bits_per_pixel = 32,
                        .red_mask = 0,
                        .green_mask = 0,
                        .blue_mask = 0
                };

                XInitImage( &img );

                pixmap = XCreatePixmap( drawable->display, drawable->root_window, destination_width, destination_height, 32 );
                graphics_context = XCreateGC( drawable->display, pixmap, 0, NULL );

                XPutImage( drawable->display, pixmap, graphics_context, &img, 0, 0, 0, 0, destination_width, destination_height );
                imlib_free_image_and_decache();
                XFreeGC( drawable->display, graphics_context );

                picture = XRenderCreatePicture( drawable->display, pixmap, XRenderFindStandardFormat( drawable->display, PictStandardARGB32 ), 0, NULL );

                XFreePixmap( drawable->display, pixmap );
        }

        return picture;
}

void draw_polygon( Internal_Drawable_t *drawable, const int x, const int y, const int original_width, const int original_height, int scaled_width, int scaled_height, const XPoint *points,
                   const int points_count, const int shape, const int filled ) {

        if ( !drawable || !drawable->scheme ) return;

        XSetForeground( drawable->display, drawable->graphics_context, drawable->scheme[ FOREGROUND ].pixel );

        // Reduces the scaled width and height by 1 when drawing the outline to compensate for X11 drawing the line 1 pixel over
        if ( !filled ) {
                scaled_width -= 1;
                scaled_height -= 1;
        }

        XPoint scaled_points[ points_count ];

        memcpy( scaled_points, points, points_count );

        for ( int v = 0; v < points_count; v++ ) {
                scaled_points[ v ] = (XPoint){ .x = (short) ( points[ v ].x * scaled_width / original_width + x ), .y = (short) ( points[ v ].y * scaled_height / original_height + y ) };
        }

        // Change shape to 'Convex' or 'Complex' if the shape is not 'Nonconvex'
        if ( filled ) {
                XFillPolygon( drawable->display, drawable->drawable, drawable->graphics_context, scaled_points, points_count, shape, CoordModeOrigin );
        } else {
                XDrawLines( drawable->display, drawable->drawable, drawable->graphics_context, scaled_points, points_count, CoordModeOrigin );
        }
}

void draw_rectangle( Internal_Drawable_t *drawable, const int x, const int y, const unsigned int width, const unsigned int height, const int filled, const int invert ) {
        if ( !drawable || !drawable->scheme ) return;

        XSetForeground( drawable->display, drawable->graphics_context, invert ? drawable->scheme[ BACKGROUND ].pixel : drawable->scheme[ FOREGROUND ].pixel );

        if ( filled ) {
                XFillRectangle( drawable->display, drawable->drawable, drawable->graphics_context, x, y, width, height );
        } else {
                XDrawRectangle( drawable->display, drawable->drawable, drawable->graphics_context, x, y, width - 1, height - 1 );
        }
}

void drawable_resize( Internal_Drawable_t *drawable, const unsigned int width, const unsigned int height ) {
        if ( !drawable ) return;

        drawable->w = width;
        drawable->h = height;

        if ( drawable->picture ) XRenderFreePicture( drawable->display, drawable->picture );
        if ( drawable->drawable ) XFreePixmap( drawable->display, drawable->drawable );

        drawable->drawable = XCreatePixmap( drawable->display, drawable->root_window, width, height, DefaultDepth( drawable->display, drawable->screen ) );
        drawable->picture = XRenderCreatePicture( drawable->display, drawable->drawable, XRenderFindVisualFormat( drawable->display, DefaultVisual( drawable->display, drawable->screen ) ), 0, NULL );
}

void drawable_set_fontset( Internal_Drawable_t *drawable, Internal_Font_t *fontset ) {
        if ( drawable ) drawable->fonts = fontset;
}

void drawable_set_scheme( Internal_Drawable_t *drawable, XftColor *scheme ) {
        if ( drawable ) drawable->scheme = scheme;
}

 // TODO: This function could use some cleaning up still. Some of the variable
 // names are still pretty contradictory or confusing. Could also likely be
 // broken up into some smaller helper functions to make this less of a monolith.
int draw_text( Internal_Drawable_t *drawable, int x, const int y, unsigned int width, const unsigned int height, const unsigned int left_padding, const char *text, const int invert ) {
        const int render = x || y || width || height;

        static const char invalid_placeholder[ ] = "�";
        static const char overflow_placeholder[ ] = "...";

        if ( !drawable || ( render && ( !drawable->scheme || !width ) ) || !text || !drawable->fonts ) return 0;

        XftDraw *xft_draw = NULL;
        if ( !render ) {
                width = invert ? invert : ~invert;
        } else {
                XSetForeground( drawable->display, drawable->graphics_context, drawable->scheme[ invert ? FOREGROUND : BACKGROUND ].pixel );
                XFillRectangle( drawable->display, drawable->drawable, drawable->graphics_context, x, y, width, height );

                if ( width < left_padding ) {
                        return x + (int) width;
                }

                xft_draw = XftDrawCreate( drawable->display, drawable->drawable, DefaultVisual( drawable->display, drawable->screen ), DefaultColormap( drawable->display, drawable->screen ) );

                x += (int) left_padding;
                width -= left_padding;
        }

        static unsigned int overflow_placeholder_width;
        if ( !overflow_placeholder_width && render ) {
                overflow_placeholder_width = drawable_fontset_getwidth( drawable, overflow_placeholder );
        }

        static unsigned int invalid_width;
        if ( !invalid_width && render ) {
                invalid_width = drawable_fontset_getwidth( drawable, invalid_placeholder );
        }

        // Not sure if I understand why this is different from `overflow_placeholder_width`,
        // don't understand this code well enough.
        unsigned int overflow_text_length;

        int overflow_text_x = 0;
        int utf8_string_length, utf8_character_length, utf8_error;
        long utf8_codepoint = 0;
        const char *utf8_string;
        Internal_Font_t *used_font = drawable->fonts;
        Internal_Font_t *current_font, *next_font;
        FcCharSet *tmp_fccharset;
        FcPattern *tmp_fcpattern;
        FcPattern *match;
        XftResult result;
        static unsigned int no_matches_list[ 128 ];
        unsigned int current_text_width, remaining_width = 0;
        bool character_exists = false, overflow = false;
        while ( 1 ) {
                current_text_width = overflow_text_length = utf8_error = utf8_character_length = utf8_string_length = 0;
                utf8_string = text;
                next_font = NULL;

                while ( *text ) {
                        utf8_character_length = utf8_decode( text, &utf8_codepoint, &utf8_error );

                        for ( current_font = drawable->fonts; current_font; current_font = current_font->next ) {
                                character_exists = character_exists || XftCharExists( drawable->display, current_font->xfont, utf8_codepoint );

                                if ( character_exists ) {
                                        unsigned int tmp_width = 0;
                                        drawable_font_get_extents( current_font, text, utf8_character_length, &tmp_width, NULL );

                                        if ( current_text_width + overflow_placeholder_width <= width ) {
                                                // keep track where the ellipsis still fits
                                                overflow_text_x = x + (int) current_text_width;
                                                remaining_width = width - current_text_width;
                                                overflow_text_length = utf8_string_length;
                                        }

                                        if ( current_text_width + tmp_width > width ) {
                                                overflow = true;

                                                // called from drw_fontset_getwidth_clamp():
                                                // it wants the width AFTER the overflow
                                                if ( !render ) {
                                                        x += (int) tmp_width;
                                                } else {
                                                        utf8_string_length = (int) overflow_text_length;
                                                }
                                        } else if ( current_font == used_font ) {
                                                text += utf8_character_length;
                                                utf8_string_length += utf8_error ? 0 : utf8_character_length;
                                                current_text_width += utf8_error ? 0 : tmp_width;
                                        } else {
                                                next_font = current_font;
                                        }

                                        break;
                                }
                        }

                        if ( overflow || !character_exists || next_font || utf8_error ) break;

                        character_exists = false;
                }

                if ( utf8_string_length ) {
                        if ( render ) {
                                const int text_y = (int) ( y + ( height - used_font->h ) / 2 + used_font->xfont->ascent );
                                XftDrawStringUtf8( xft_draw, &drawable->scheme[ invert ? BACKGROUND : FOREGROUND ], used_font->xfont, x, text_y, (XftChar8 *) utf8_string, utf8_string_length );
                        }

                        x += (int) current_text_width;
                        width -= current_text_width;
                }

                if ( utf8_error && ( !render || invalid_width < width ) ) {
                        if ( render ) {
                                draw_text( drawable, x, y, width, height, 0, invalid_placeholder, invert );
                        }

                        x += (int) invalid_width;
                        width -= invalid_width;
                }

                if ( render && overflow ) {
                        draw_text( drawable, overflow_text_x, y, remaining_width, height, 0, overflow_placeholder, invert );
                }

                if ( !*text || overflow ) {
                        break;
                }

                if ( next_font ) {
                        character_exists = false;
                        used_font = next_font;
                } else {
                        // Regardless of whether a fallback font
                        // is found, the character must be drawn.
                        character_exists = true;

                        unsigned int hash = (unsigned int) utf8_codepoint;
                        hash = ( ( hash >> 16 ) ^ hash ) * 0x21F0AAAD;
                        hash = ( ( hash >> 15 ) ^ hash ) * 0xD35A2D97;

                        const unsigned int h0 = ( ( hash >> 15 ) ^ hash ) % LENGTH( no_matches_list );
                        const unsigned int h1 = ( hash >> 17 ) % LENGTH( no_matches_list );

                        // avoid expensive XftFontMatch call when we know we won't find a match
                        if ( no_matches_list[ h0 ] == utf8_codepoint || no_matches_list[ h1 ] == utf8_codepoint ) goto no_match;

                        tmp_fccharset = FcCharSetCreate();
                        FcCharSetAddChar( tmp_fccharset, utf8_codepoint );

                        if ( !drawable->fonts->pattern ) {
                                // Refer to the comment in xfont_create for more information.
                                log_fatal( "The first font in the cache must be loaded from a font string" );
                        }

                        tmp_fcpattern = FcPatternDuplicate( drawable->fonts->pattern );
                        FcPatternAddCharSet( tmp_fcpattern, FC_CHARSET, tmp_fccharset );
                        FcPatternAddBool( tmp_fcpattern, FC_SCALABLE, FcTrue );

                        FcConfigSubstitute( NULL, tmp_fcpattern, FcMatchPattern );
                        FcDefaultSubstitute( tmp_fcpattern );
                        match = XftFontMatch( drawable->display, drawable->screen, tmp_fcpattern, &result );

                        FcCharSetDestroy( tmp_fccharset );
                        FcPatternDestroy( tmp_fcpattern );

                        if ( match ) {
                                used_font = xfont_create( drawable, NULL, match );
                                if ( used_font && XftCharExists( drawable->display, used_font->xfont, utf8_codepoint ) ) {
                                        for ( current_font = drawable->fonts; current_font->next; current_font = current_font->next ) {} /* NOP */
                                        current_font->next = used_font;
                                } else {
                                        xfont_free( used_font );
                                        no_matches_list[ no_matches_list[ h0 ] ? h1 : h0 ] = utf8_codepoint;
                                no_match:
                                        used_font = drawable->fonts;
                                }
                        }
                }
        }

        if ( xft_draw ) {
                XftDrawDestroy( xft_draw );
        }

        return (int) ( x + ( render ? width : 0 ) );
}

// TODO: Make some of these variable names more readable.
int utf8_decode( const char *s_in, long *u, int *err ) {

        const long utf_invalid = 0xFFFD;

        // @formatter:off
	static const unsigned char lens[] = {
		/* 0XXXX */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		/* 10XXX */ 0, 0, 0, 0, 0, 0, 0, 0,  /* invalid */
		/* 110XX */ 2, 2, 2, 2,
		/* 1110X */ 3, 3,
		/* 11110 */ 4,
		/* 11111 */ 0,  /* invalid */
	};
        // @formatter:on

        static const unsigned char leading_mask[ ] = { 0x7F, 0x1F, 0x0F, 0x07 };
        static const unsigned int overlong[ ] = { 0x0, 0x80, 0x0800, 0x10000 };

        const unsigned char *s = (const unsigned char *) s_in;
        const int len = lens[ *s >> 3 ];
        *u = utf_invalid;

        *err = 1;
        if ( len == 0 ) return 1;

        long cp = s[ 0 ] & leading_mask[ len - 1 ];
        for ( int i = 1; i < len; ++i ) {
                if ( s[ i ] == '\0' || ( s[ i ] & 0xC0 ) != 0x80 ) return i;
                cp = ( cp << 6 ) | ( s[ i ] & 0x3F );
        }

        /* out of range, surrogate, overlong encoding */
        if ( cp > 0x10FFFF || ( cp >> 11 ) == 0x1B || cp < overlong[ len - 1 ] ) return len;

        *err = 0;
        *u = cp;
        return len;
}

Internal_Font_t *xfont_create( const Internal_Drawable_t *drawable, const char *font_name, FcPattern *font_pattern ) {
        XftFont *xfont = NULL;
        FcPattern *pattern = NULL;

        if ( font_name ) {
                // Using the pattern found at font->xfont->pattern does not yield the
                // same substitution results as using the pattern returned by
                // FcNameParse; using the latter results in the desired fallback
                // behaviour whereas the former just results in missing-character
                // rectangles being drawn, at least with some fonts.
                if ( !( ( xfont = XftFontOpenName( drawable->display, drawable->screen, font_name ) ) ) ) {
                        log_error( "Cannot load font from name: '%s'", font_name );
                        return NULL;
                }

                if ( !( ( pattern = FcNameParse( (FcChar8 *) font_name ) ) ) ) {
                        log_error( "Cannot parse font name to pattern: '%s'", font_name );
                        XftFontClose( drawable->display, xfont );
                        return NULL;
                }
        } else if ( font_pattern ) {
                if ( !( ( xfont = XftFontOpenPattern( drawable->display, font_pattern ) ) ) ) {
                        log_error( "Cannot load font from pattern" );
                        return NULL;
                }
        } else {
                log_fatal( "No font specified" );
        }

        Internal_Font_t *font = ecalloc( 1, sizeof( Internal_Font_t ) );

        font->xfont = xfont;
        font->pattern = pattern;
        font->h = xfont->ascent + xfont->descent;
        font->display = drawable->display;

        return font;
}

void xfont_free( Internal_Font_t *font ) {
        if ( !font ) return;
        if ( font->pattern ) FcPatternDestroy( font->pattern );
        XftFontClose( font->display, font->xfont );
        free( font );
}

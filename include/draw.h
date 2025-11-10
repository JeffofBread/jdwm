#ifndef DRW_H_
#define DRW_H_

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/render.h>
#include <fontconfig/fontconfig.h>

#define TEXT_WIDTH( X ) ( drawable_fontset_getwidth( g_drawable, ( X ) ) + g_left_right_text_padding )

enum Cursors {
        CURSOR_NORMAL,
        CURSOR_RESIZE,
        CURSOR_MOVE,
        CURSOR_COUNT
};

enum Color_Indices {

        // Bar section themes
        FOREGROUND = 0,
        BACKGROUND = 1,

        // Client themes
        NORMAL_BORDER   = 0,
        FLOATING_BORDER = 1,
};

// TODO: Restructure for readability and padding
typedef struct Internal_Drawable {
        int screen;
        unsigned int w, h;
        Display *display;
        Window root_window;
        Drawable drawable;
        Picture picture;
        GC graphics_context;
        XftColor *scheme;
        struct Internal_Font *fonts;
} Internal_Drawable_t;

// TODO: Restructure for readability and padding
typedef struct Internal_Font {
        unsigned int h;
        Display *display;
        XftFont *xfont;
        FcPattern *pattern;
        struct Internal_Font *next;
} Internal_Font_t;

extern int drawable_color_create( Internal_Drawable_t *drawable, XftColor *destination, const char *color_hex );
extern Internal_Drawable_t *drawable_create( Display *display, int screen, Window root_window, unsigned int width, unsigned int height );
extern Cursor *drawable_cursor_create( Internal_Drawable_t *drawable, int shape );
extern void drawable_cursor_free( const Internal_Drawable_t *drawable, Cursor *cursor );
extern Internal_Font_t *drawable_font_create( Internal_Drawable_t *drawable, char fonts[ ] );
extern void drawable_fontset_free( Internal_Font_t *fontset );
extern unsigned int drawable_fontset_getwidth( Internal_Drawable_t *drawable, const char *text );
extern void drawable_font_get_extents( Internal_Font_t *font, const char *text, unsigned int length, unsigned int *width, unsigned int *height );
extern void drawable_free( Internal_Drawable_t *drawable );
extern void draw_map( Internal_Drawable_t *drawable, Window window, int x, int y, unsigned int width, unsigned int height );
extern void draw_picture( Internal_Drawable_t *drawable, int x, int y, unsigned int width, unsigned int height, Picture picture );
extern Picture drawable_picture_create_resized( const Internal_Drawable_t *drawable, char *source, unsigned int source_width, unsigned int source_height, unsigned int destination_width,
                                                unsigned int destination_height );
extern void draw_polygon( Internal_Drawable_t *drawable, int x, int y, int original_width, int original_height, int scaled_width, int scaled_height, const XPoint *points, int points_count, int shape,
                          int filled );
extern void draw_rectangle( Internal_Drawable_t *drawable, int x, int y, unsigned int width, unsigned int height, int filled, int invert );
extern void drawable_resize( Internal_Drawable_t *drawable, unsigned int width, unsigned int height );
extern void drawable_set_fontset( Internal_Drawable_t *drawable, Internal_Font_t *fontset );
extern void drawable_set_scheme( Internal_Drawable_t *drawable, XftColor *scheme );
extern int draw_text( Internal_Drawable_t *drawable, int x, int y, unsigned int width, unsigned int height, unsigned int left_padding, const char *text, int invert );
extern int utf8_decode( const char *s_in, long *u, int *err );
extern Internal_Font_t *xfont_create( const Internal_Drawable_t *drawable, const char *font_name, FcPattern *font_pattern );
extern void xfont_free( Internal_Font_t *font );

extern Internal_Drawable_t *g_drawable;

#endif /* DRW_H_ */

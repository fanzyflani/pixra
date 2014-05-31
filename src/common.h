/*
Copyright (c) 2014 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <math.h>

#include <zlib.h>
#include <SDL.h>

#include <assert.h>

#include <signal.h>

typedef enum
{
	UNDO_INVALID = 0,

	UNDO_PENCIL,
	UNDO_SETPAL,
	UNDO_RESIZE,

	UNDO_COUNT
} undo_type_t;

typedef union undo undo_t;
union undo
{
	undo_type_t typ;

	// UNDO_PENCIL
	union {
		undo_type_t typ;
		uint32_t ncol;
		uint32_t sx, sy;
		uint32_t sw, sh;
		uint32_t ocolsize;
		uint32_t *bits; // (bits[x>>5]>>(x&31))&1 sw*sh
		uint8_t *ocollist;
	} pencil;

	// UNDO_SETPAL
	union {
		undo_type_t typ;
		uint32_t idx;
		uint32_t ocol;
		uint32_t ncol;
	} setpal;

	// UNDO_RESIZE
	union {
		undo_type_t typ;
		// TODO!
	} resize;
};

typedef struct img
{
	// Image stuff
	int w, h;
	uint32_t pal[256];
	uint16_t dpal[4][256];
	uint8_t *data;

	// Image state
	int dirty;
	int zoom, zx, zy;
	uint8_t *ldata;
	undo_t *undo_top;
	undo_t *undo_bottom;
	size_t undosz_total;
	size_t undosz_bottom;
} img_t;

typedef struct widget widget_t;
struct widget
{
	widget_t *parent, *child, *sibp, *sibn;
	int w, h;
	int x, y; // Don't access these during draw - they are parent-relative
	void (*f_free)(widget_t *g);
	void (*f_draw)(widget_t *g, int sx, int sy);
	void (*f_pack)(widget_t *g, int w, int h);
	void (*f_mouse_pass)(widget_t *g, int focused);
	void (*f_mouse_button)(widget_t *g, int mx, int my, int button, int state);
	void (*f_mouse_motion)(widget_t *g, int mx, int my, int dx, int dy, int bail, int buttons);
	void *v1;
};

#define SCR16(x, y) ((x) + (uint16_t *)(screen->pitch * (y) + (uint8_t *)(screen->pixels)))
#define IMG16(img, x, y) ((x) + (uint16_t *)(img->w * (y) + (uint8_t *)(img->data)))
#define IMG8(img, x, y)  ((x) +  (uint8_t *)(img->w * (y) + (uint8_t *)(img->data)))

// TODO: refactor these into some GUI toolkit
#define EJUSTX(x) (screen->w - x)
#define EJUSTY(y) (screen->h - y)

#define W_IMG_X1 132
#define W_IMG_Y1 12
#define W_IMG_X2 EJUSTX(12)
#define W_IMG_Y2 EJUSTY(12)

#define W_PAL_X1 0
#define W_PAL_Y1 0
#define W_PAL_X2 (0+128)
#define W_PAL_Y2 (12+512)

// conv.c
extern const uint32_t dtab[];
uint32_t rgba32(int r, int g, int b, int a);
uint32_t rgb32(int r, int g, int b);
uint16_t rgb16(int r, int g, int b);
uint16_t c32to16(uint32_t c, int x, int y);

// draw.c
void draw_rect32(int x1, int y1, int x2, int y2, uint32_t col);
void draw_img(img_t *img, int zoom, int sx, int sy, int dx, int dy, int sw, int sh);

// img.c
void img_undirty(img_t *img);
img_t *img_new(int w, int h);

// tool.c

// widget.c
void widget_reparent(widget_t *parent, widget_t *child);
int widget_mouse_motion(int x, int y, int dx, int dy, int buttons, widget_t *g);
int widget_mouse_motion_sdl(SDL_Event *ev, int bx, int by, widget_t *g);
int widget_mouse_button(int x, int y, int button, int state, widget_t *g);
int widget_mouse_button_sdl(SDL_Event *ev, int bx, int by, widget_t *g);
void widget_free(widget_t *g);
widget_t *widget_new(widget_t *parent, int x, int y, int w, int h, widget_t *(*f_init)(widget_t *g));
widget_t *w_cpick_init(widget_t *g);
widget_t *w_pal_init(widget_t *g);
widget_t *w_img_init(widget_t *g);
widget_t *w_desk_init(widget_t *g);

// main.c
extern SDL_Surface *screen;
extern img_t *rootimg;
extern widget_t *rootg;
extern int tool_palidx;
extern int mouse_x;
extern int mouse_y;
extern widget_t *g_img;
extern widget_t *g_pal;
extern widget_t *g_cpick;


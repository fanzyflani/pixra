/*
Copyright (c) 2014 mrokei & contributors.
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

typedef struct window window_t;
struct window
{
	int w, h, x, y;
	void *v1;
};

#define SCR16(x, y) ((x) + (uint16_t *)(screen->pitch * (y) + (uint8_t *)(screen->pixels)))
#define IMG16(img, x, y) ((x) + (uint16_t *)(img->w * (y) + (uint8_t *)(img->data)))
#define IMG8(img, x, y)  ((x) +  (uint8_t *)(img->w * (y) + (uint8_t *)(img->data)))

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

// main.c
extern SDL_Surface *screen;
extern img_t *rootimg;


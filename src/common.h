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
} img_t;

#define SCR16(x, y) ((x) + (uint16_t *)(screen->pitch * (y) + (uint8_t *)(screen->pixels)))
#define IMG16(img, x, y) ((x) + (uint16_t *)(img->w * (y) + (uint8_t *)(img->data)))
#define IMG8(img, x, y)  ((x) +  (uint8_t *)(img->w * (y) + (uint8_t *)(img->data)))


/*
Copyright (c) 2014 mrokei & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

void img_undirty(img_t *img)
{
	int i, j;

	// Check
	if(img->dirty == 0)
		return;

	// Dither palette
	for(i = 0; i < 4; i++)
	for(j = 0; j < 256; j++)
		img->dpal[i][j] = c32to16(img->pal[j], i&1, i>>1);

	// Clear
	img->dirty = 0;
}

img_t *img_new(int w, int h)
{
	int x, y, i;

	img_t *img = malloc(sizeof(img_t));

	// Image stuff
	img->w = w;
	img->h = h;

	// Image state
	img->dirty = 1;
	img->zoom = 8;
	img->zx = 0;
	img->zy = 0;
	img->undo_top = NULL;
	img->undo_bottom = NULL;
	img->undosz_total = 0;
	img->undosz_bottom = 0;

	// Image palette
#if 0
	for(i = 0; i < 256; i++)
	{
		int hi = i;
		int si = 0;
		int vi = 0;

		float r = sin(M_PI*hi*3.0/128.0);
		float g = sin(M_PI*hi*3.0/128.0 + M_PI*2.0/3.0);
		float b = sin(M_PI*hi*3.0/128.0 + M_PI*4.0/3.0);

		// TODO: Oscillate these
		float s = 1.0;
		float v = 1.0;

		r = v*((s*r + (1.0-s))*0.5 + 0.5);
		g = v*((s*g + (1.0-s))*0.5 + 0.5);
		b = v*((s*b + (1.0-s))*0.5 + 0.5);

		r = r*255.0 + 0.5;
		g = g*255.0 + 0.5;
		b = b*255.0 + 0.5;

		img->pal[i] = rgb32(r, g, b);
	}
#endif
	for(i = 0; i < 256; i++)
		img->pal[i] = rgb32(0xFF, 0, 0xFF);

	// EGA + grey 16 + websafe
	for(i = 0; i < 16; i++)
	{
		//
		int r = (i&4 ? 170 : 0);
		int g = (i&2 ? 170 : 0);
		int b = (i&1 ? 170 : 0);
		int intens = (i&8 ? 85 : 0); 

		if(i == 6) g = 85;

		img->pal[i] = rgb32(r|intens, g|intens, b|intens);
	}

	for(i = 0; i < 16; i++)
	{
		int c = (i+1)*0x0F;
		img->pal[i + 16] = rgb32(c, c, c);
	}

	for(i = 0; i < 216; i++)
	{
		int r = (i/36)*0x33;
		int g = ((i/6)%6)*0x33;
		int b = (i%6)*0x33;

		img->pal[i + 32] = rgb32(r, g, b);
	}

	// Image data
	img->data = malloc(w*h);
	img->ldata = malloc(w*h);

	// TODO: Actually *clear* this
	for(y = 0; y < h; y++)
	for(x = 0; x < w; x++)
		*IMG8(img, x, y) = x^y;
	
	// Image return
	return img;
}


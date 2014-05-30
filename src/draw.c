/*
Copyright (c) 2014 mrokei & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

void draw_rect32(int x1, int y1, int x2, int y2, uint32_t col)
{
	int x, y;
	int w, h, pitch;
	uint16_t dl[2][2];
	uint16_t *dest;

	// Enforce x1 <= x2, y1 <= y2.
	//
	// I cannot be bothered throwing in a temporary variable,
	// so I'm doing a subtract ladder instead.
	//
	// This does a swap, by the way.
	// Don't believe me? Try the math yourself.
	// TODO: confirm if -= is safe wrt wraparound! may need ^= instead
	if(x1 > x2) { x1 -= x2; x2 -= x1; x1 -= x2; }
	if(y1 > y2) { y1 -= y2; y2 -= y1; y1 -= y2; }

	// Bail out if off-screen.
	if(x2 < 0 || y2 < 0) return;
	if(x1 >= screen->w || y1 >= screen->h) return;

	// Add to x2,y2 to make it exclusive.
	x2++; y2++;

	// Pad to fit.
	if(x1 < 0) x1 = 0;
	if(x2 > screen->w) x2 = screen->w;
	if(y1 < 0) y1 = 0;
	if(y2 > screen->h) y2 = screen->h;

	// Calculate width, height, pitch.
	w = x2 - x1;
	h = y2 - y1;
	pitch = (screen->pitch>>1) - w;

	// Build dither table.
	dl[0][0] = c32to16(col, x1+0, y1+0);
	dl[0][1] = c32to16(col, x1+1, y1+0);
	dl[1][0] = c32to16(col, x1+0, y1+1);
	dl[1][1] = c32to16(col, x1+1, y1+1);

	// Draw!
	dest = SCR16(x1, y1);
	for(y = 0; y < h; y++, dest += pitch)
	{
		uint16_t *dla = dl[y&1];

		for(x = 0; x < w; x++)
			*(dest++) = dla[x&1];
	}
}

void draw_img(img_t *img, int zoom, int sx, int sy, int dx, int dy, int sw, int sh)
{
	int x, y, tx, ty;
	int dw, dh, dp;

	// Undirty the image
	img_undirty(img);

	// Clip the destination coordinates.
	// TODO: Actually clip and not just bail
	if(dx < 0) return;
	if(dy < 0) return;
	if(dx + sw*zoom > screen->w) return;
	if(dy + sh*zoom > screen->h) return;

	// Move the source coordinates along if negative.
	if(sx < 0) { dx -= zoom*sx; sw += sx; sx = 0; }
	if(sy < 0) { dy -= zoom*sy; sh += sy; sy = 0; }

	// Clip the source dimensions to suit.
	if(sx + sw > img->w) { sw = img->w - sx; }
	if(sy + sh > img->h) { sh = img->h - sy; }

	// Calculate width and height.
	dw = sw * zoom;
	dh = sh * zoom;

	// Give up if coordinates out of range or width/height useless.
	if(sx >= img->w || sy >= img->h) return;
	if(dx >= screen->w || dy >= screen->h) return;
	if(dw < zoom || dh < zoom) return;

	// Draw the image.
	// TODO: Specialised per-level zoom
	dp = screen->pitch >> 1;
	for(y = 0; y < sh; y++)
	{
		uint8_t *src = IMG8(img, sx, y + sy);
		uint16_t *dest = SCR16(dx, dy + y*zoom);

		for(x = 0; x < sw; x++)
		{
			uint32_t sv = *(src++);

			for(tx = 0; tx < zoom; tx++, dest++)
				for(ty = 0; ty < zoom; ty++)
					dest[ty * dp] = img->dpal[(tx&1)+((ty&1)<<1)][sv];
		}
	}
}


/*
Copyright (c) 2014, 2016 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

void draw_rect32(int x1, int y1, int x2, int y2, uint32_t col)
{
	int x, y;
	int w, h, pitch;
#if SCREEN_BPP == 16
	uint16_t dl[2][2];
	uint16_t *dest;
#endif
#if SCREEN_BPP == 32
	uint32_t *dest;
#endif

	// Enforce x1 <= x2, y1 <= y2.
	//
	// I cannot be bothered throwing in a temporary variable,
	// so I'm doing an XOR ladder instead.
	//
	// This does a swap, by the way.
	// Don't believe me? Try the math yourself.
	if(x1 > x2) { x1 ^= x2; x2 ^= x1; x1 ^= x2; }
	if(y1 > y2) { y1 ^= y2; y2 ^= y1; y1 ^= y2; }

	// Bail out if off-screen.
	if(x2 < 0 || y2 < 0) return;
	if(x1 >= screen_w || y1 >= screen_h) return;

	// Add to x2,y2 to make it exclusive.
	x2++; y2++;

	// Pad to fit.
	if(x1 < 0) x1 = 0;
	if(x2 > screen_w) x2 = screen_w;
	if(y1 < 0) y1 = 0;
	if(y2 > screen_h) y2 = screen_h;

	// Calculate width, height, pitch.
	w = x2 - x1;
	h = y2 - y1;

#if SCREEN_BPP == 16
	pitch = (screen_pitch>>1) - w;

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
#endif
#if SCREEN_BPP == 32
	pitch = (screen_pitch>>2) - w;

	// Draw!
	dest = SCR32(x1, y1);
	for(y = 0; y < h; y++, dest += pitch)
		for(x = 0; x < w; x++)
			*(dest++) = col;
#endif
}

void draw_rect8_img(img_t *img, int x1, int y1, int x2, int y2, uint8_t col)
{
	int x, y;
	int w, h, pitch;
	uint8_t *dest;

	// TODO: Set up a common clip framework.

	// Enforce x1 <= x2, y1 <= y2.
	//
	// I cannot be bothered throwing in a temporary variable,
	// so I'm doing an XOR ladder instead.
	//
	// This does a swap, by the way.
	// Don't believe me? Try the math yourself.
	if(x1 > x2) { x1 ^= x2; x2 ^= x1; x1 ^= x2; }
	if(y1 > y2) { y1 ^= y2; y2 ^= y1; y1 ^= y2; }

	// Bail out if off-screen.
	if(x2 < 0 || y2 < 0) return;
	if(x1 >= img->w || y1 >= img->h) return;

	// Add to x2,y2 to make it exclusive.
	x2++; y2++;

	// Pad to fit.
	if(x1 < 0) x1 = 0;
	if(x2 > img->w) x2 = img->w;
	if(y1 < 0) y1 = 0;
	if(y2 > img->h) y2 = img->h;

	// Calculate width, height, pitch.
	w = x2 - x1;
	h = y2 - y1;
	pitch = (img->w) - w;

	// Draw!
	dest = IMG8(img, x1, y1);
	for(y = 0; y < h; y++, dest += pitch)
		for(x = 0; x < w; x++)
			*(dest++) = col;
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
	if(dx + sw*zoom > screen_w) return;
	if(dy + sh*zoom > screen_h) return;

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
	if(dx >= screen_w || dy >= screen_h) return;
	if(dw < zoom || dh < zoom) return;

	// Draw the image.
	// TODO: Specialised per-level zoom
#if SCREEN_BPP == 16
	dp = screen_pitch >> 1;
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
#endif
#if SCREEN_BPP == 32
	dp = screen_pitch >> 2;
	for(y = 0; y < sh; y++)
	{
		uint8_t *src = IMG8(img, sx, y + sy);
		uint32_t *dest = SCR32(dx, dy + y*zoom);

		for(x = 0; x < sw; x++)
		{
			uint32_t sv = *(src++);

			for(tx = 0; tx < zoom; tx++, dest++)
				for(ty = 0; ty < zoom; ty++)
					dest[ty * dp] = img->pal[sv];
		}
	}
#endif
}

void draw_img_trans(img_t *img, int zoom, int sx, int sy, int dx, int dy, int sw, int sh, uint8_t tcol)
{
	int x, y, tx, ty;
	int dw, dh, dp;

	// Undirty the image
	img_undirty(img);

	// Clip the destination coordinates.
	// TODO: Actually clip and not just bail
	if(dx < 0) return;
	if(dy < 0) return;
	if(dx + sw*zoom > screen_w) return;
	if(dy + sh*zoom > screen_h) return;

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
	if(dx >= screen_w || dy >= screen_h) return;
	if(dw < zoom || dh < zoom) return;

	// Draw the image.
	// TODO: Specialised per-level zoom
#if SCREEN_BPP == 16
	dp = screen_pitch >> 1;
	for(y = 0; y < sh; y++)
	{
		uint8_t *src = IMG8(img, sx, y + sy);
		uint16_t *dest = SCR16(dx, dy + y*zoom);

		for(x = 0; x < sw; x++)
		{
			uint32_t sv = *(src++);

			if(sv != tcol)
			{
				for(tx = 0; tx < zoom; tx++, dest++)
					for(ty = 0; ty < zoom; ty++)
						dest[ty * dp] = img->dpal[(tx&1)+((ty&1)<<1)][sv];

			} else {
				dest += zoom;

			}
		}
	}
#endif
#if SCREEN_BPP == 32
	dp = screen_pitch >> 2;
	for(y = 0; y < sh; y++)
	{
		uint8_t *src = IMG8(img, sx, y + sy);
		uint32_t *dest = SCR32(dx, dy + y*zoom);

		for(x = 0; x < sw; x++)
		{
			uint32_t sv = *(src++);

			if(sv != tcol)
			{
				for(tx = 0; tx < zoom; tx++, dest++)
					for(ty = 0; ty < zoom; ty++)
						dest[ty * dp] = img->pal[sv];

			} else {
				dest += zoom;

			}
		}
	}
#endif
}

void draw_printf(int dx, int dy, int zoom, uint16_t c, const char *fmt, ...)
{
	va_list va;
	char buf[1024];

	// Firstly, make sure we actually *have* a font loaded.
	if(fontimg == NULL) return;

	// Secondly, make sure dx, dy are in range.
	if(dx + 8*zoom > screen_w || dy + 8*zoom > screen_h || dy < 0) return;

	// Now format the string.
	va_start(va, fmt);
	vsnprintf(buf, 1023, fmt, va);
	buf[1023] = '\x00';

	// Finally, start drawing it.
	// TODO: Support multiple colours!
	uint8_t *cp = (uint8_t *)buf;
	for(; *cp != '\x00' && dx + zoom*8 <= screen_w; cp++, dx += zoom*8)
	{
		// Check if X in range
		if(dx < 0) continue;

		// Get source char position
		int sx = (*cp)&15;
		int sy = (*cp)>>4;
		sx <<= 3;
		sy <<= 3;

		// Draw
		// TODO: Use a simpler routine
		draw_img_trans(fontimg, zoom, sx, sy, dx, dy, 8, 8, 0);
	}
}

static void draw_floodfill_img_step(img_t *img, int dx, int dy, uint8_t ocol, uint8_t ncol)
{
	int sx1, sx2;

	// Ensure we have the floodfill parameters correct
	assert(*IMG8(img, dx, dy) == ocol);

	// Move to left of span
	while(dx >= 1 && *IMG8(img, dx-1, dy) == ocol) dx--;
	sx1 = dx;
	
	// Move to right of span, changing colours
	uint8_t *cp = IMG8(img, dx, dy);
	while(dx < img->w && *cp == ocol)
	{
		*(cp++) = ncol;
		dx++;
	}
	sx2 = dx-1;

	// Check top and bottom for spans
	// We've already filled this span, so this should be simple.
	for(dx = sx1; dx <= sx2; dx++)
	{
		if(dy >= 1 && *IMG8(img, dx, dy-1) == ocol)
			draw_floodfill_img_step(img, dx, dy-1, ocol, ncol);
		if(dy+1 < img->h && *IMG8(img, dx, dy+1) == ocol)
			draw_floodfill_img_step(img, dx, dy+1, ocol, ncol);
	}
}

void draw_floodfill_img(img_t *img, int dx, int dy, uint8_t col)
{
	uint8_t scol;
	
	// Check if in range
	if(dx < 0 || dy < 0 || dx >= img->w || dy >= img->h) return;

	// Get source colour and check that it doesn't match our current colour
	// This is because our algorithm requires the colours to be different.
	scol = *IMG8(img, dx, dy);
	if(scol == col) return;

	// Perform the floodfill.
	draw_floodfill_img_step(img, dx, dy, scol, col);
}



/*
Copyright (c) 2014 mrokei & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

static const uint32_t dtab[4] = {
	0, 4,
	6, 2,
};

SDL_Surface *screen = NULL;

img_t *rootimg = NULL;

uint32_t rgba32(int r, int g, int b, int a)
{
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;
	if(a < 0) a = 0;

	if(r > 0xFF) r = 0xFF;
	if(g > 0xFF) g = 0xFF;
	if(b > 0xFF) b = 0xFF;
	if(a > 0xFF) a = 0xFF;

	return (a<<24) | (r<<16) | (g<<8) | b;
}

uint32_t rgb32(int r, int g, int b)
{
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

	if(r > 0xFF) r = 0xFF;
	if(g > 0xFF) g = 0xFF;
	if(b > 0xFF) b = 0xFF;

	return 0xFF000000 | (r<<16) | (g<<8) | b;
}

uint16_t rgb16(int r, int g, int b)
{
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

	r >>= 3;
	g >>= 2;
	b >>= 3;

	if(r > 0x1F) r = 0x1F;
	if(g > 0x3F) g = 0x3F;
	if(b > 0x1F) b = 0x1F;

	return (r<<11) | (g<<5) | b;
}

uint16_t c32to16(uint32_t c, int x, int y)
{
	uint32_t r = (c>>16) & 255;
	uint32_t g = (c>>8)  & 255;
	uint32_t b = (c   )  & 255;

	int d = dtab[(x&1)+((y&1)<<1)];

	r += d;   if(r > 255) r = 255;
	g += d>>1;if(g > 255) g = 255;
	b += d;   if(b > 255) b = 255;

	r >>= 3;
	g >>= 3;
	b >>= 3;

	return (r<<11) | (g<<5) | b;
}

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

	// Image palette
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

	// Image data
	img->data = malloc(w*h);

	// TODO: Actually *clear* this
	for(y = 0; y < h; y++)
	for(x = 0; x < w; x++)
		*IMG8(img, x, y) = x^y;
	
	// Image return
	return img;
}

void img_draw(img_t *img, int zoom, int sx, int sy, int dx, int dy, int sw, int sh)
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
		uint16_t *dest = SCR16(dx, dy + y*zoom + ty);

		for(x = 0; x < sw; x++)
		{
			uint32_t sv = *(src++);

			for(tx = 0; tx < zoom; tx++, dest++)
				for(ty = 0; ty < zoom; ty++)
					dest[ty * dp] = img->dpal[(tx&1)+((ty&1)<<1)][sv];
		}
	}
}

void mainloop(void)
{
	int i;

	// Clear screen
	SDL_LockSurface(screen);
	memset(screen->pixels, 0, screen->pitch * screen->h);

	// Draw image
	img_draw(rootimg, rootimg->zoom,
		rootimg->zx, rootimg->zy,
		0, 0,
		screen->w/rootimg->zoom, screen->h/rootimg->zoom);

	// Draw palette
	for(i = 0; i < 256; i++)
	{
		*SCR16(2*i+0, 0) = rootimg->dpal[0][i];
		*SCR16(2*i+1, 0) = rootimg->dpal[1][i];
		*SCR16(2*i+0, 1) = rootimg->dpal[2][i];
		*SCR16(2*i+1, 1) = rootimg->dpal[3][i];
	}

	// Blit
	SDL_UnlockSurface(screen);
	SDL_Flip(screen);

	// TODO: Input
	SDL_Delay(1000);
}

int main(int argc, char *argv[])
{
	// Init SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	// Correct SDL's stupid signal eating thing (who the hell hooks SIGTERM like that?!)
	signal(SIGINT,  SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	// Set video mode
	// Avoid dwm's autoresize - do it twice
	SDL_WM_SetCaption("pixra - fast paint tool", NULL);
	screen = SDL_SetVideoMode(800, 600, 16, 0);
	screen = SDL_SetVideoMode(800, 600, 16, SDL_RESIZABLE);

	// Set stuff up
	rootimg = img_new(400, 300);

	// Main loop!
	mainloop();

	// Clean up and go bye bye
	return 0;
}


/*
Copyright (c) 2014 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

SDL_Surface *screen = NULL;
img_t *rootimg = NULL;

// TODO: refactor
int tool_palidx = 0;

void mainloop_draw(void)
{
	int i;

	// Clear screen
	SDL_LockSurface(screen);
	memset(screen->pixels, 0, screen->pitch * screen->h);

	// Draw image
	draw_img(rootimg, rootimg->zoom,
		rootimg->zx, rootimg->zy,
		W_IMG_X1, W_IMG_Y1,
		(W_IMG_X2 - W_IMG_X1)/rootimg->zoom,
		(W_IMG_Y2 - W_IMG_Y1)/rootimg->zoom);

	// Draw palette
	for(i = 0; i < 256; i++)
	{
		draw_rect32(
			W_PAL_X1 + ((i&7)<<4),
			W_PAL_Y1 + ((i>>3)<<4),
			W_PAL_X1 + ((i&7)<<4)+15,
			W_PAL_Y1 + ((i>>3)<<4)+15,
		rootimg->pal[i]);
	}

	// Blit
	SDL_UnlockSurface(screen);
	SDL_Flip(screen);
}

void mainloop(void)
{
	SDL_Event ev;
	int quitflag = 0;
	int x, y, i;

	while(quitflag == 0)
	{
		// Draw
		mainloop_draw();

		// Delay
		SDL_Delay(10);

		// Get input
		while(SDL_PollEvent(&ev))
		switch(ev.type)
		{
			case SDL_QUIT:
				quitflag = 1;
				break;

#define MB_INRANGE(x1,y1,x2,y2) (ev.button.x >= (x1) && ev.button.x < (x2) \
	&& ev.button.y >= (y1) && ev.button.y < (y2))
			case SDL_MOUSEBUTTONDOWN:
				if(MB_INRANGE(W_IMG_X1, W_IMG_Y1, W_IMG_X2, W_IMG_Y2))
				{
					// Screen -> Widget mapping
					x = ev.button.x - W_IMG_X1;
					y = ev.button.y - W_IMG_Y1;

					// Widget -> Image mapping
					x /= rootimg->zoom;
					y /= rootimg->zoom;
					x += rootimg->zx;
					y += rootimg->zy;
					
					// TODO: Deal with the issue where (w, h) % rootimg->zoom != 0

					// Put a pixel somewhere
					*IMG8(rootimg, x, y) = tool_palidx;
					rootimg->dirty = 1; // TODO: Several "dirty" flags

				} else if(MB_INRANGE(W_PAL_X1, W_PAL_Y1, W_PAL_X2, W_PAL_Y2)) {
					// Screen -> Widget mapping
					x = ev.button.x - W_PAL_X1;
					y = ev.button.y - W_PAL_Y1;

					// Widget -> Palette mapping
					tool_palidx = (x>>4) + ((y>>4)<<3);
				}
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	// Init SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	// Correct SDL's stupid signal eating thing (who the hell hooks SIGTERM like that?!)
	signal(SIGINT,  SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	// Set video mode
	SDL_WM_SetCaption("pixra - fast paint tool", NULL);
	screen = SDL_SetVideoMode(800, 600, 16, 0);

	// Set stuff up
	rootimg = img_new(400, 300);

	// Main loop!
	mainloop();

	// Clean up and go bye bye
	return 0;
}


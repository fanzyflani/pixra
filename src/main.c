/*
Copyright (c) 2014 mrokei & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

SDL_Surface *screen = NULL;
img_t *rootimg = NULL;

void mainloop(void)
{
	int i;

	// Clear screen
	SDL_LockSurface(screen);
	memset(screen->pixels, 0, screen->pitch * screen->h);

	// Draw image
	draw_img(rootimg, rootimg->zoom,
		rootimg->zx, rootimg->zy,
		130, 12,
		(screen->w - 130 - 1)/rootimg->zoom, (screen->h - 12 - 12)/rootimg->zoom);

	// Draw palette
	for(i = 0; i < 256; i++)
	{
		draw_rect32(
			0  + ((i&7)<<4),
			12 + ((i>>3)<<4),
			0  + ((i&7)<<4)+15,
			12 + ((i>>3)<<4)+15,
		rootimg->pal[i]);
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
	SDL_WM_SetCaption("pixra - fast paint tool", NULL);
	screen = SDL_SetVideoMode(800, 600, 16, 0);

	// Set stuff up
	rootimg = img_new(400, 300);

	// Main loop!
	mainloop();

	// Clean up and go bye bye
	return 0;
}


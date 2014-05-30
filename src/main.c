/*
Copyright (c) 2014 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

SDL_Surface *screen = NULL;
img_t *rootimg = NULL;
widget_t *rootg = NULL;

// TODO: refactor
int tool_palidx = 0;

widget_t *g_img = NULL;
widget_t *g_pal = NULL;

void mainloop_draw(void)
{
	// Clear screen
	SDL_LockSurface(screen);
	memset(screen->pixels, 0, screen->pitch * screen->h);

	// Draw stuff
	g_img->f_draw(g_img, g_img->x, g_img->y);
	g_pal->f_draw(g_pal, g_pal->x, g_pal->y);

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

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				0
				|| widget_mouse_button(&ev, 0, 0, g_pal)
				|| widget_mouse_button(&ev, 0, 0, g_img)
				;
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
	g_pal = widget_new(NULL, W_PAL_X1, W_PAL_Y1, W_PAL_X2 - W_PAL_X1, W_PAL_Y2 - W_PAL_Y1, w_pal_init);
	g_img = widget_new(NULL, W_IMG_X1, W_IMG_Y1, W_IMG_X2 - W_IMG_X1, W_IMG_Y2 - W_IMG_Y1, w_img_init);

	// Main loop!
	mainloop();

	// Clean up and go bye bye
	return 0;
}


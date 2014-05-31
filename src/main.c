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

int mouse_x = 0;
int mouse_y = 0;

void mainloop_draw(void)
{
	// Clear screen
	SDL_LockSurface(screen);
	memset(screen->pixels, 0, screen->pitch * screen->h);

	// Draw stuff
	rootg->f_draw(rootg, rootg->x, rootg->y);

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

			case SDL_MOUSEMOTION:
				mouse_x = ev.motion.x;
				mouse_y = ev.motion.y;
				widget_mouse_motion_sdl(&ev, 0, 0, rootg);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				mouse_x = ev.button.x;
				mouse_y = ev.button.y;
				widget_mouse_button_sdl(&ev, 0, 0, rootg);
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
	rootg = widget_new(NULL, 0, 0, screen->w, screen->h, w_desk_init);
	g_img = widget_new(rootg, W_IMG_X1, W_IMG_Y1, W_IMG_X2 - W_IMG_X1, W_IMG_Y2 - W_IMG_Y1, w_img_init);
	g_pal = widget_new(rootg, W_PAL_X1, W_PAL_Y1, W_PAL_X2 - W_PAL_X1, W_PAL_Y2 - W_PAL_Y1, w_pal_init);

	// Main loop!
	mainloop();

	// Clean up and go bye bye
	return 0;
}


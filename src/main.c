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
widget_t *g_cpick = NULL;

int key_mods = 0;
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

void handle_key(int key, int state)
{
	int maskset = 0;

	// Check for modifier keys
	if(0) ;
	else if(key == SDLK_LCTRL) maskset = KM_LCTRL;
	else if(key == SDLK_RCTRL) maskset = KM_RCTRL;
	else if(key == SDLK_LSHIFT) maskset = KM_LSHIFT;
	else if(key == SDLK_RSHIFT) maskset = KM_RSHIFT;

	// Apply modifiers
	if(state) key_mods |=  maskset;
	else /**/ key_mods &= ~maskset;

	// Do other keys
	if(maskset != 0) return;

	if(!state)
	switch(key)
	{
		case SDLK_s:
			if((key_mods & KM_CTRL))
				img_save_tga(rootimg->fname, rootimg);

			break;

		case SDLK_l:
			if((key_mods & KM_CTRL))
			{
				img_t *img = img_load_tga(rootimg->fname);

				if(img != NULL)
				{
					img_free(rootimg);
					rootimg = img;
				}
			}

			break;
	}
}

void mainloop(void)
{
	SDL_Event ev;
	int quitflag = 0;

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

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				handle_key(ev.key.keysym.sym, ev.type == SDL_KEYDOWN);
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
	// Read arguments
	if(argc <= 3)
	{
		printf("usage:\n\t%s width height filename.tga\n", argv[0]);
		return 99;
	}

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	if(w <= 0 || h <= 0)
	{
		printf("ERROR: invalid dimensions %ix%i\n", w, h);
		return 1;
	}

	const char *fname = argv[3];

	// Init SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	// Correct SDL's stupid signal eating thing (who the hell hooks SIGTERM like that?!)
	signal(SIGINT,  SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	// Set video mode
	SDL_WM_SetCaption("pixra - fast paint tool", NULL);
	screen = SDL_SetVideoMode(800, 600, 16, 0);

	// Set up image
	rootimg = NULL;
	if(rootimg == NULL) rootimg = img_load_tga(fname);
	if(rootimg == NULL) rootimg = img_new(w, h);
	if(rootimg->fname == NULL) rootimg->fname = strdup(fname);

	// Set up GUI
	rootg = widget_new(NULL, 0, 0, screen->w, screen->h, w_desk_init);
	g_img = widget_new(rootg, W_IMG_X1, W_IMG_Y1, W_IMG_X2 - W_IMG_X1, W_IMG_Y2 - W_IMG_Y1, w_img_init);
	g_pal = widget_new(rootg, W_PAL_X1, W_PAL_Y1, W_PAL_X2 - W_PAL_X1, W_PAL_Y2 - W_PAL_Y1, w_pal_init);
	g_cpick = widget_new(NULL, 0, 0, 512+2*4, 20*3+4*4, w_cpick_init);

	// Main loop!
	mainloop();

	// Clean up and go bye bye
	return 0;
}


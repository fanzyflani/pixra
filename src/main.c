/*
Copyright (c) 2014 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

#ifdef WIN32
// for MessageBox and some path crap
#include <windows.h>
#endif

SDL_Surface *screen = NULL;
img_t *rootimg = NULL;
img_t *clipimg = NULL;
img_t *fontimg = NULL;
widget_t *rootg = NULL;

// TODO: refactor
int tool_palidx = 0;
int tool_bgidx = 0;

int tool_cx1 = -1;
int tool_cy1 = -1;
int tool_cx2 = -1;
int tool_cy2 = -1;

int tool_gx = -1;
int tool_gy = -1;
int tool_gw = -1;
int tool_gh = -1;

int tool_pe1 = -1;
int tool_pe2 = -1;

int tool_pasting = 0;

widget_t *g_img = NULL;
widget_t *g_pal = NULL;
widget_t *g_cpick = NULL;

int key_mods = 0;
int key_mods_drag = 0;
int mouse_x = 0;
int mouse_y = 0;
int mouse_b = 0;

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

	// Update drag modifiers if no mouse buttons held
	if(mouse_b == 0) key_mods_drag = key_mods;

	// Do other keys
	if(maskset != 0) return;

	if(!state)
	switch(key)
	{
		case SDLK_ESCAPE:
			if(!key_mods)
			{
				// Cancel stuff

				// Colour picker window
				if(g_cpick->parent != NULL)
					widget_reparent(NULL, g_cpick);

				// Paste
				if(tool_pasting)
					tool_pasting = 0;

			}
			break;

		case SDLK_b:
			if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL))
			{if(clipimg != NULL)
			{
				// Paste
				tool_pasting = 1;

			}}

			break;

		case SDLK_c:
			if(!key_mods)
			{
				// Colour picker window
				if(g_cpick->parent == NULL)
				{
					// Set cpick position
					g_cpick->x = mouse_x - g_cpick->w/2;
					g_cpick->y = mouse_y - g_cpick->h/2;

					// Move if touching offscreen
					if(g_cpick->x < 0) { g_cpick->x = 0; }
					if(g_cpick->y < 0) { g_cpick->y = 0; }
					if(g_cpick->x + g_cpick->w > screen->w) {
						g_cpick->x = screen->w - g_cpick->w; }
					if(g_cpick->y + g_cpick->h > screen->h) {
						g_cpick->y = screen->h - g_cpick->h; }

					// Reparent
					widget_reparent(rootg, g_cpick);
				} else {
					// Deparent
					widget_reparent(NULL, g_cpick);
				}

			} else if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL)) {
				// Copy image
				img_t *img = img_copy(rootimg, tool_cx1, tool_cy1, tool_cx2, tool_cy2);
				if(img != NULL)
				{
					// Replace clipboard image
					if(clipimg != NULL) img_free(clipimg);
					clipimg = img;
					printf("Copied\n");
				}

			}

			break;

		case SDLK_g:
			if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL))
			{if(tool_cx1 != -1 && tool_cx2 != -1)
			{
				// Set grid according to box select
				// Get bounds in order
				int x1 = (tool_cx1 < tool_cx2 ? tool_cx1 : tool_cx2);
				int y1 = (tool_cy1 < tool_cy2 ? tool_cy1 : tool_cy2);
				int x2 = (tool_cx1 > tool_cx2 ? tool_cx1 : tool_cx2);
				int y2 = (tool_cy1 > tool_cy2 ? tool_cy1 : tool_cy2);

				// Calculate width, height
				tool_gw = (x2 - x1) + 1;
				tool_gh = (y2 - y1) + 1;

				// Calculate corner
				tool_gx = x1 % tool_gw;
				tool_gy = y1 % tool_gh;
			}}

			break;


		case SDLK_l:
			if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL))
			{
				// Push undo step
				img_push_undo(rootimg);

				// Load
				img_t *img = img_load_tga(rootimg->fname);

				if(img != NULL)
				{
					img_free(rootimg);
					rootimg = img;
				}
			}

			break;

		case SDLK_r:
			if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL))
			{if(tool_cx1 != -1 && tool_cx2 != -1)
			{
				// Rect fill
				draw_rect8_img(rootimg, tool_cx1, tool_cy1, tool_cx2, tool_cy2, tool_palidx);

			}}

			break;

		case SDLK_s:
			if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL))
			{
				// Save
				img_save_tga(rootimg->fname, rootimg);
			}

			break;

		case SDLK_v:
			if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL))
			{if(clipimg != NULL)
			{
				// Paste
				tool_pasting = 2;

			}}

			break;

		case SDLK_x:
			if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL))
			{if(tool_cx1 != -1 && tool_cx2 != -1)
			{
				// Cut image
				img_t *img = img_copy(rootimg, tool_cx1, tool_cy1, tool_cx2, tool_cy2);
				if(img != NULL)
				{
					// Push undo step
					img_push_undo(rootimg);

					// Replace clipboard image
					if(clipimg != NULL) img_free(clipimg);
					clipimg = img;
					printf("Copied (Cut)\n");

					// Now draw rectangle
					draw_rect8_img(rootimg,
						tool_cx1, tool_cy1,
						tool_cx2, tool_cy2,
						tool_bgidx);
				}

			}}

			break;

		case SDLK_z:
			if((key_mods & KM_CTRL) && !(key_mods & ~KM_CTRL))
			{
				// Undo
				if(rootimg->undo != NULL)
				{
					rootimg = rootimg->undo;
					rootimg->dirty = 1;
				}

			} else if((key_mods & (KM_CTRL | KM_SHIFT)) && !(key_mods & ~(KM_CTRL | KM_SHIFT))) {
				// Redo
				if(rootimg->redo != NULL)
				{
					rootimg = rootimg->redo;
					rootimg->dirty = 1;
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
				// Update mouse_[xy]
				mouse_x = ev.motion.x;
				mouse_y = ev.motion.y;

				// Call
				widget_mouse_motion_sdl(&ev, 0, 0, rootg);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				// Update mouse_b button bitmask (IF NOT SCROLLWHEEL)
				if(ev.button.button-1 == 3 || ev.button.button-1 == 4)
					;
				else if(ev.type == SDL_MOUSEBUTTONDOWN)
					mouse_b |= 1<<(ev.button.button-1);
				else
					mouse_b &= ~(1<<(ev.button.button-1));

				// Update drag mods if we now have no buttons down
				if(mouse_b == 0) key_mods_drag = key_mods;

				// Update mouse_[xy]
				mouse_x = ev.button.x;
				mouse_y = ev.button.y;

				// Call
				widget_mouse_button_sdl(&ev, 0, 0, rootg);
				break;
		}
	}
}

char newbie_fnbuf[2048] = "";
char newbie_wbuf[7] = "320";
char newbie_hbuf[7] = "200";
int newbie_selidx = 0;
int newbie_showmsg = 200;
char newbie_msg[256] = "Enter filename (*.tga format), and if it doesn't exist, enter width and height";

void newbieloop_draw(void)
{
	// Clear screen
	SDL_LockSurface(screen);
	memset(screen->pixels, 0, screen->pitch * screen->h);

	// Draw stuff
#define NEWBIE_ISSEL(x) (newbie_selidx == (x) ? '>' : ' ')
	draw_printf(0, 0, 2, rgb16(255, 255, 255), "%c File: %s_",NEWBIE_ISSEL(0), newbie_fnbuf);
	draw_printf(0, 16, 2, rgb16(255, 255, 255), "%c Width: %s_", NEWBIE_ISSEL(1), newbie_wbuf);
	draw_printf(0, 32, 2, rgb16(255, 255, 255), "%c Height: %s_", NEWBIE_ISSEL(2), newbie_hbuf);
	draw_printf(0, 48, 2, rgb16(255, 255, 255), "%c OK", NEWBIE_ISSEL(3));

	// Draw message if need be
	if(newbie_showmsg > 0)
	{
		draw_printf(0, 80, 1, rgb16(255, 255, 255), "%s", newbie_msg);
		newbie_showmsg--;
	}

	// Blit
	SDL_UnlockSurface(screen);
	SDL_Flip(screen);
}

int newbieloop(void)
{
	SDL_Event ev;
	int quitflag = 0;

	while(quitflag == 0)
	{
		// Draw
		newbieloop_draw();

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
			switch(ev.key.keysym.sym)
			{
				case SDLK_UP:
					newbie_selidx--;
					if(newbie_selidx < 0)
						newbie_selidx = 3;
					break;

				case SDLK_DOWN:
					newbie_selidx++;
					if(newbie_selidx > 3)
						newbie_selidx = 0;
					break;

				case SDLK_BACKSPACE: {
					if(newbie_selidx >= 3) break;

					char *p = (newbie_selidx == 0 ? newbie_fnbuf
						: newbie_selidx == 1 ? newbie_wbuf
						: newbie_hbuf);
					//int ml = (newbie_selidx == 0 ? 2048 : 7);
					int cl = strlen(p);

					if(cl != 0) p[cl-1] = '\x00';
				} break;

				case SDLK_RETURN: {
					// Attempt to use the given args.

					// Check filename for validity
					if(strlen(newbie_fnbuf) == 0)
					{
						snprintf(newbie_msg, 255, "We need a filename! Type one in!");
						newbie_msg[255] = '\x00';
						newbie_showmsg = 200;
						break;
					}

					// Now try LOADING
					rootimg = img_load_tga(newbie_fnbuf);

					if(rootimg != NULL)
						return 0;

					// Failing that, try CREATING
					int w = atoi(newbie_wbuf);
					int h = atoi(newbie_hbuf);

					if(w < 1 || h < 1)
					{
						snprintf(newbie_msg, 255, "Invalid dimensions %i x %i", w, h);
						newbie_msg[255] = '\x00';
						newbie_showmsg = 200;
						break;
					}

					rootimg = img_new(w, h);
					rootimg->fname = strdup(newbie_fnbuf);
					return 0;

				} break;

				default: {
					if(newbie_selidx >= 3) break;

					char *p = (newbie_selidx == 0 ? newbie_fnbuf
						: newbie_selidx == 1 ? newbie_wbuf
						: newbie_hbuf);
					int ml = (newbie_selidx == 0 ? 2048 : 7);
					int cl = strlen(p);

					if(ev.key.keysym.unicode >= 32 && ev.key.keysym.unicode < 127)
					{
						if(cl+1 == ml) break;

						p[cl] = (char)ev.key.keysym.unicode;
						p[cl+1] = '\x00';
					}

				} break;
			} break;

			case SDL_MOUSEMOTION:
				// TODO!
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				// TODO!
				break;
		}
	}

	return (quitflag == -1 ? 0 : quitflag);
}

int main(int argc, char *argv[])
{
	// Read arguments
	if(argc-1 != 1 && argc-1 != 3)
	{
		printf("usage:\n\t%s filename.tga [width height]\n", argv[0]);
		if(argc-1 != 0)
			return 99;
	}

	const char *fname = (argc > 1 ? argv[1] : NULL);

	// Load font
	char fontfnbuf[2048] = "";
#ifdef HARD_DAT_DIR
#ifdef WIN32
	strcpy(fontfnbuf, HARD_DAT_DIR "\\dat\\font8.tga");
#else
	strcpy(fontfnbuf, HARD_DAT_DIR "/dat/font8.tga");
#endif
#else
#ifdef WIN32
	GetModuleFileName(NULL, fontfnbuf, 2047);
	fontfnbuf[2047] = '\x00';

	// No, I'm not adding shlwapi.
	char *folfont = fontfnbuf + strlen(fontfnbuf);
	for(; folfont >= fontfnbuf; folfont--)
	if(*folfont == '\\')
	{
		// Truncate
		*folfont = '\x00';
		break;
	}

	strncat(fontfnbuf, "\\dat\\font8.tga", 2047 - strlen(fontfnbuf));

#else
	strncpy(fontfnbuf, argv[0], 2048);
	fontfnbuf[2047] = '\x00';

	// There are two possibilities here.
	// 1. It was launched globally. Thus, we pretend our files are in /usr/local/share/pixra/.
	//    (Technically you should compile this with HARD_DAT_DIR set!)
	// 2. It was launched from some directory. Thus, we follow along.
	//    (If you relied on #1, note that it's now not going to work!)

	// Why do we use this method? Because, well...
	// - Linux has /proc/self/exe.
	// - FreeBSD has /proc/curproc/file, but only if procfs is mounted.
	// - NetBSD I think has /proc/self/exe but only if something's mounted.
	//   I don't quite remember what I needed to do, it's been a while.

	// Seeing as I'm devving on FreeBSD and Raspbian Linux,
	// I don't want too much platform-specific crap *between* the two.
	// But hey, Windows insists on making people do platform-specific crap.

	// *sigh* I hate Windows.

	char *folfont = fontfnbuf + strlen(fontfnbuf);
	for(; folfont >= fontfnbuf; folfont--)
	if(*folfont == '/')
	{
		// Truncate
		*folfont = '\x00';
		break;
	}

	if(folfont < fontfnbuf)
		strcpy(fontfnbuf, "/usr/local/share/pixra/font8.tga");
	else
		strncat(fontfnbuf, "/dat/font8.tga", 2047 - strlen(fontfnbuf));
#endif
#endif
	fontimg = img_load_tga(fontfnbuf);
	if(fontimg == NULL)
		printf("Couldn't load %s! No text will be displayed.\n", fontfnbuf);

	if(fontimg == NULL && fname == NULL)
	{
		char *const message = ""
			"Uh oh! pixra can't find its font!\r\n"
			"This program can run without it,\r\n"
			"but the 'main menu' GUI requires it.\r\n"
			"\r\n"
			"You will need to either:\r\n"
			"1. Make it so pixra can find a 'dat/font8.tga' file, or\r\n"
			"2. Run pixra in the commandline.\r\n"
			"\r\n"
			"See the USAGE.md file for the commandline arguments.\r\n"
		;

		printf("----\n%s----\n", message);

#ifdef WIN32
		MessageBox(NULL, message, "pixra", MB_OK | MB_APPLMODAL | MB_ICONSTOP);
#else
		char *const msgargs[] = {"/usr/bin/env", "xmessage", message, NULL};
		execv("/usr/bin/env", msgargs);
#endif

		return 1;
	}

	// Set up image
	rootimg = NULL;
	if(rootimg == NULL && fname != NULL) rootimg = img_load_tga(fname);
	if(rootimg == NULL && fname != NULL)
	{
		if(argc <= 3)
		{
			printf("To create a new image, specify width and height after the filename\n");
			return 1;
		}

		int w = atoi(argv[2]);
		int h = atoi(argv[3]);
		if(w <= 0 || h <= 0)
		{
			printf("ERROR: invalid dimensions %ix%i\n", w, h);
			return 1;
		}

		rootimg = img_new(w, h);
	}

	if(rootimg != NULL && rootimg->fname == NULL) rootimg->fname = strdup(fname);

	// Init SDL
	// SDL_INIT_TIMER has been dropped due to it breaking valgrind on FreeBSD
	SDL_Init(SDL_INIT_VIDEO);
	SDL_EnableUNICODE(1);

	// Correct SDL's stupid signal eating thing (who the hell hooks SIGTERM like that?!)
	signal(SIGINT,  SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	// Set video mode
	SDL_WM_SetCaption("pixra - fast paint tool", NULL);
	screen = SDL_SetVideoMode(800, 600, 16, 0);

	// Loop for "newbie" menu.
	if(rootimg == NULL && fname == NULL)
		if(newbieloop())
			return 1;

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


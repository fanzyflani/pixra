/*
Copyright (c) 2014 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

void widget_reparent(widget_t *parent, widget_t *child)
{
	// Detach child from parent
	if(child->parent != NULL && child->parent->child == child)
		child->parent->child = child->sibn;

	// Detach child from siblings
	if(child->sibp != NULL) { child->sibp->sibn = child->sibn; }
	if(child->sibn != NULL) { child->sibn->sibp = child->sibp; }

	// Attach to new parent
	child->parent = parent;
	child->sibp = NULL;

	if(parent == NULL)
	{
		child->sibn = NULL;

	} else {
		child->sibn = parent->child;

		if(parent->child != NULL)
			parent->child->sibp = child;

		parent->child = child;
	}
}

int widget_mouse_motion(int x, int y, int dx, int dy, int buttons, widget_t *g)
{
	int bail = 0;

	x -= g->x;
	y -= g->y;

	// Check if we were in the widget
	x -= dx; y -= dy;
	if(x < 0 || y < 0 || x >= g->w || y >= g->h)
		return 0;

	// Check if we are still in the widget, or if we are bailing
	x += dx; y += dy;
	if(x < 0 || y < 0 || x >= g->w || y >= g->h)
		bail = 1;

	// Call our function
	if(g->f_mouse_motion != NULL)
		g->f_mouse_motion(g, x, y, dx, dy, bail, buttons);

	return 1;
}

int widget_mouse_motion_sdl(SDL_Event *ev, int bx, int by, widget_t *g)
{
	int x = ev->motion.x - bx;
	int y = ev->motion.y - by;
	int dx = ev->motion.xrel;
	int dy = ev->motion.yrel;
	int buttons = ev->motion.state;

	return widget_mouse_motion(x, y, dx, dy, buttons, g);
}

int widget_mouse_button(int x, int y, int button, int state, widget_t *g)
{
	x -= g->x;
	y -= g->y;

	if(x < 0 || y < 0 || x >= g->w || y >= g->h)
		return 0;

	if(g->f_mouse_button != NULL)
		g->f_mouse_button(g, x, y, button, state);

	return 1;
}

int widget_mouse_button_sdl(SDL_Event *ev, int bx, int by, widget_t *g)
{
	int x = ev->button.x - bx;
	int y = ev->button.y - by;
	int button = ev->button.button - 1;
	int state = (ev->type == SDL_MOUSEBUTTONDOWN ? 1 : 0);

	return widget_mouse_button(x, y, button, state, g);
}

// WARNING: Frees children as well!
// If you don't want this to happen, reparent them with widget_reparent!
void widget_free(widget_t *g)
{
	widget_t *p = g->child;
	widget_t *p2;

	// Free children
	while(p != NULL)
	{
		p2 = p->sibn;
		widget_free(p);
		p = p2;
	}

	// Orphan this widget
	widget_reparent(NULL, g);

	// Free self
	if(g->f_free != NULL) { g->f_free(g); }
	free(g);
}

widget_t *widget_new(widget_t *parent, int x, int y, int w, int h, widget_t *(*f_init)(widget_t *g))
{
	// Allocate
	widget_t *g = malloc(sizeof(widget_t));

	// Coords
	g->x = x;
	g->y = y;
	g->w = w;
	g->h = h;

	// Variables
	g->v1 = NULL;

	// Links
	g->parent = parent;
	g->child = NULL;
	g->sibp = NULL;
	g->sibn = NULL;
	widget_reparent(parent, g);

	// Functions
	g->f_free = NULL;
	g->f_draw = NULL;
	g->f_pack = NULL;
	g->f_mouse_pass = NULL;
	g->f_mouse_button = NULL;
	g->f_mouse_motion = NULL;

	return f_init(g);
}

//
// PALETTE WIDGET
//
static void w_pal_draw(widget_t *g, int sx, int sy)
{
	int i;

	for(i = 0; i < 256; i++)
	{
		draw_rect32(
			sx + ((i&7)<<4),
			sy + ((i>>3)<<4),
			sx + ((i&7)<<4)+15,
			sy + ((i>>3)<<4)+15,
		rootimg->pal[i]);
	}

	draw_rect32(sx, sy + 512, sx + 127, sy + 512 + 15, rgb32(255, 255, 255));
	draw_rect32(sx + 2, sy + 512 + 2, sx + 127 - 2, sy + 512 + 15 - 2, rootimg->pal[tool_palidx]);

}

static void w_pal_pack(widget_t *g, int w, int h)
{
	g->w = 128;
	g->h = 512 + 16;
}

static void w_pal_mouse_button(widget_t *g, int mx, int my, int button, int state)
{
	if(!state) return;
	if(button != 0) return;

	// Widget -> Palette mapping
	tool_palidx = (mx>>4) + ((my>>4)<<3);

}

widget_t *w_pal_init(widget_t *g)
{
	//
	g->f_draw = w_pal_draw;
	g->f_pack = w_pal_pack;
	g->f_mouse_button = w_pal_mouse_button;

	return g;
}

//
// IMAGE WIDGET
//
static void w_img_draw(widget_t *g, int sx, int sy)
{
	draw_img(rootimg, rootimg->zoom,
		rootimg->zx, rootimg->zy,
		sx, sy,
		g->w/rootimg->zoom,
		g->h/rootimg->zoom);
}

static void w_img_pack(widget_t *g, int w, int h)
{
	g->w = w;
	g->h = h;
}

static void w_img_mouse_button(widget_t *g, int mx, int my, int button, int state)
{
	if(!state) return;
	if(button != 0) return;

	int x = mx;
	int y = my;

	// Widget -> Image mapping
	x /= rootimg->zoom;
	y /= rootimg->zoom;
	x += rootimg->zx;
	y += rootimg->zy;
	
	// TODO: Deal with the issue where (w, h) % rootimg->zoom != 0

	// Put a pixel somewhere
	if(x >= 0 && y >= 0 && x < rootimg->w && y < rootimg->h)
	{
		*IMG8(rootimg, x, y) = tool_palidx;
		rootimg->dirty = 1; // TODO: Several "dirty" flags
	}
}

static void w_img_mouse_motion_lmb(widget_t *g, int mx, int my, int dx, int dy, int bail)
{
	int i;

	int x = mx;
	int y = my;
	int lx = x - dx;
	int ly = y - dy;

	// Widget -> Image mapping
	x /= rootimg->zoom;
	y /= rootimg->zoom;
	x += rootimg->zx;
	y += rootimg->zy;

	// Widget -> Image mapping for old position
	lx /= rootimg->zoom;
	ly /= rootimg->zoom;
	lx += rootimg->zx;
	ly += rootimg->zy;
	
	// TODO: Deal with the issue where (w, h) % rootimg->zoom != 0

	// Put a pixel somewhere
	if(lx != x || ly != y)
	{
		// Prepare line variables
		int dcx = x - lx;
		int dcy = y - ly;
		int dsx = (dcx < 0 ? -1 : 1);
		int dsy = (dcy < 0 ? -1 : 1);
		dcx *= dsx;
		dcy *= dsy;

		int dc = dcx-dcy;

		for(i = 0; i < dcx+dcy; i++)
		{
			// Advance counter
			if(dc >= 0) { dc -= dcy; lx += dsx; }
			else /* */ { dc += dcx; ly += dsy; }

			if(lx >= 0 && ly >= 0 && lx < rootimg->w && ly < rootimg->h)
				*IMG8(rootimg, lx, ly) = tool_palidx;
		}

		rootimg->dirty = 1; // TODO: Several "dirty" flags
	}

	// If bailing, warp the mouse
	// TODO: Make this trigger BEFORE the border
	if(bail)
	{
		// Get image position
		int mdx = (mx >= (g->w>>1) ? -1 : 1) * (g->w>>1);
		int mdy = (my >= (g->h>>1) ? -1 : 1) * (g->h>>1);
		mdx /= rootimg->zoom;
		mdy /= rootimg->zoom;

		// TODO: Clamp to image bounds

		// Move image and warp mouse
		rootimg->zx -= mdx;
		rootimg->zy -= mdy;
		SDL_WarpMouse(mouse_x + mdx*rootimg->zoom, mouse_y + mdy*rootimg->zoom);
	}
}

static void w_img_mouse_motion_mmb(widget_t *g, int mx, int my, int dx, int dy, int bail)
{
	int x = mx;
	int y = my;
	int lx = x - dx;
	int ly = y - dy;

	// Widget -> Image mapping
	x /= rootimg->zoom;
	y /= rootimg->zoom;
	x += rootimg->zx;
	y += rootimg->zy;

	// Widget -> Image mapping for old position
	lx /= rootimg->zoom;
	ly /= rootimg->zoom;
	lx += rootimg->zx;
	ly += rootimg->zy;

	// Scroll
	rootimg->zx -= x - lx;
	rootimg->zy -= y - ly;

	// If bailing, warp the mouse
	// TODO: Make this trigger BEFORE the border
	if(bail)
	{
		// Get image position
		int mdx = (mx >= (g->w>>1) ? -1 : 1) * (g->w>>1);
		int mdy = (my >= (g->h>>1) ? -1 : 1) * (g->h>>1);
		mdx /= rootimg->zoom;
		mdy /= rootimg->zoom;

		// TODO: Clamp to image bounds

		// Warp mouse
		SDL_WarpMouse(mouse_x + mdx*rootimg->zoom, mouse_y + mdy*rootimg->zoom);
	}

}

static void w_img_mouse_motion(widget_t *g, int mx, int my, int dx, int dy, int bail, int buttons)
{
	if((buttons & 1) != 0) return w_img_mouse_motion_lmb(g, mx, my, dx, dy, bail);
	if((buttons & 2) != 0) return w_img_mouse_motion_mmb(g, mx, my, dx, dy, bail);
}

widget_t *w_img_init(widget_t *g)
{
	g->f_draw = w_img_draw;
	g->f_pack = w_img_pack;
	g->f_mouse_button = w_img_mouse_button;
	g->f_mouse_motion = w_img_mouse_motion;

	return g;
}

//
// DESKTOP CONTAINER WIDGET
//
static void w_desk_draw(widget_t *g, int sx, int sy)
{
	widget_t *child;

	// Try to obtain child
	child = g->child;
	if(child == NULL)
		return;

	// Move to end of list
	while(child->sibn != NULL)
		child = child->sibn;

	// Traverse children BTF
	for(; child != NULL; child = child->sibp)
	{
		if(child->f_draw != NULL)
			child->f_draw(child, sx + child->x, sy + child->y);
	}
}

static void w_desk_pack(widget_t *g, int w, int h)
{
	g->w = w;
	g->h = h;
}

static void w_desk_mouse_button(widget_t *g, int mx, int my, int button, int state)
{
	widget_t *child;

	// Traverse children
	for(child = g->child; child != NULL; child = child->sibn)
		if(widget_mouse_button(mx, my, button, state, child))
			return;
}

static void w_desk_mouse_motion(widget_t *g, int mx, int my, int dx, int dy, int bail, int buttons)
{
	widget_t *child;

	// Traverse children
	for(child = g->child; child != NULL; child = child->sibn)
		if(widget_mouse_motion(mx, my, dx, dy, buttons, child))
			return;
}

widget_t *w_desk_init(widget_t *g)
{
	g->f_draw = w_desk_draw;
	g->f_pack = w_desk_pack;
	g->f_mouse_button = w_desk_mouse_button;
	g->f_mouse_motion = w_desk_mouse_motion;

	return g;
}


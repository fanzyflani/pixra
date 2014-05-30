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
		parent->child = child;
	}
}

int widget_mouse_button(SDL_Event *ev, int bx, int by, widget_t *g)
{
	int x = ev->button.x - bx - g->x;
	int y = ev->button.y - by - g->y;
	int button = ev->button.button - 1;
	int state = (ev->type == SDL_MOUSEBUTTONDOWN ? 1 : 0);
	
	if(x < 0 || y < 0 || x >= g->w || y >= g->h)
		return 0;

	if(g->f_mouse_button != NULL)
		g->f_mouse_button(g, x, y, button, state);

	return 1;
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
	g->f_mouse_move = NULL;

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

}

static void w_pal_pack(widget_t *g, int w, int h)
{
	g->w = 128;
	g->h = 512;
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
	*IMG8(rootimg, x, y) = tool_palidx;
	rootimg->dirty = 1; // TODO: Several "dirty" flags

}

widget_t *w_img_init(widget_t *g)
{
	//
	g->f_draw = w_img_draw;
	g->f_pack = w_img_pack;
	g->f_mouse_button = w_img_mouse_button;

	return g;
}


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
	g->f_key = NULL;

	return f_init(g);
}

//
// COLOUR PICKER WIDGET
//
static void w_cpick_draw(widget_t *g, int sx, int sy)
{
	int x, y;

	draw_rect32(sx, sy, sx + g->w-1, sy + g->h-1, rgb32(85, 85, 85));

	for(y = 0; y < 3; y++)
	for(x = 0; x < 256; x++)
	{
		// NOTE: could be faster - this is a kinda slow call!
		// Hmm, perhaps we could prep an image for this?

		draw_rect32(
			sx +  2*x + 4,
			sy + 24*y + 4,
			sx +  2*x + 5,
			sy + 24*y + 23,
			0xFF000000 | (x<<(y<<3)));
	}

}

static void w_cpick_pack(widget_t *g, int w, int h)
{
	// TODO: Sort this out properly
	g->w = 512+2*4;
	g->h = 20*3+4*4;
}

static void w_cpick_mouse_motion(widget_t *g, int mx, int my, int dx, int dy, int bail, int buttons)
{
#if 0
	if(bail)
		widget_reparent(NULL, g);
#endif
}

static void w_cpick_mouse_button(widget_t *g, int mx, int my, int button, int state)
{
	if(!state) return;
	if(button != 0) return;

	// Widget -> Colour mapping
	mx -= 4;
	my -= 4;

	if((my % 24) >= 20) return;

	mx /= 2;
	my /= 24;
	if(mx < 0 || mx >= 256) return;
	if(my < 0 || my >= 3) return;

	// Change colour
	uint32_t *pp = &rootimg->pal[tool_palidx];

	switch(my)
	{
		case 0:
			*pp = (*pp & ~0x000000FF) | mx;
			rootimg->dirty = 1;
			break;

		case 1:
			*pp = (*pp & ~0x0000FF00) | (mx<<8);
			rootimg->dirty = 1;
			break;

		case 2:
			*pp = (*pp & ~0x00FF0000) | (mx<<16);
			rootimg->dirty = 1;
			break;
	}

}

widget_t *w_cpick_init(widget_t *g)
{
	//
	g->f_draw = w_cpick_draw;
	g->f_pack = w_cpick_pack;
	g->f_mouse_button = w_cpick_mouse_button;
	g->f_mouse_motion = w_cpick_mouse_motion;

	return g;
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
	draw_rect32(sx + 2, sy + 512 + 2, sx + 63, sy + 512 + 15 - 2, rootimg->pal[tool_palidx]);
	draw_rect32(sx + 64, sy + 512 + 2, sx + 127 - 2, sy + 512 + 15 - 2, rootimg->pal[tool_bgidx]);

}

static void w_pal_pack(widget_t *g, int w, int h)
{
	g->w = 128;
	g->h = 512 + 16;
}

static void w_pal_mouse_button(widget_t *g, int mx, int my, int button, int state)
{
	// Widget -> Palette mapping
	int idx = (mx>>4) + ((my>>4)<<3);

	// Check type
	if(button == 0 && state)
	{
		// LMB down: Pick drawing colour
		tool_palidx = idx;
	} else if(button == 2 && state) {
		// RMB down: Pick transparent colour
		tool_bgidx = idx;
	}
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
	int x, y;

	// Draw background
	draw_rect32(sx, sy, sx+g->w-1, sy+g->h-1, rgb32(0x22, 0x22, 0x22));

	// Draw image
	draw_img(rootimg, rootimg->zoom,
		rootimg->zx, rootimg->zy,
		sx, sy,
		g->w/rootimg->zoom,
		g->h/rootimg->zoom);

	// Draw clipboard image (if pasting)
	if(tool_pasting) do
	{
		// Transform coordinates
		int px1 = ((mouse_x - g->x)/rootimg->zoom);
		int py1 = ((mouse_y - g->y)/rootimg->zoom);
		int px2 = px1 + clipimg->w;
		int py2 = py1 + clipimg->h;

		// Create clip boundary
		int cpx1 = px1;
		int cpy1 = py1;
		int cpx2 = px2;
		int cpy2 = py2;

		// No really, create the *boundary*
		if(cpx1 < 0) cpx1 = 0;
		if(cpy1 < 0) cpy1 = 0;
		if(cpx2 > g->w/rootimg->zoom) cpx2 = g->w/rootimg->zoom;
		if(cpy2 > g->h/rootimg->zoom) cpy2 = g->h/rootimg->zoom;

		// Bail out if out of camera range
		if(cpx1 >= px2 || cpy1 >= py2 || cpx2 <= px1 || cpy2 <= py1)
			break;

		// Draw
		if(tool_pasting == 2)
			draw_img_trans(clipimg, rootimg->zoom,
				cpx1 - px1,
				cpy1 - py1,
				cpx1 * rootimg->zoom + sx,
				cpy1 * rootimg->zoom + sy,
				cpx2 - cpx1,
				cpy2 - cpy1,
				tool_bgidx);
		else
			draw_img(clipimg, rootimg->zoom,
				cpx1 - px1,
				cpy1 - py1,
				cpx1 * rootimg->zoom + sx,
				cpy1 * rootimg->zoom + sy,
				cpx2 - cpx1,
				cpy2 - cpy1);

	} while(0);

	// Draw light grid
	if(rootimg->zoom >= 3) do
	{
		// Transform grid bounds
		int gfx = (0 - rootimg->zx) * rootimg->zoom;
		int gfy = (0 - rootimg->zy) * rootimg->zoom;
		int glx = (rootimg->w - rootimg->zx) * rootimg->zoom;
		int gly = (rootimg->h - rootimg->zy) * rootimg->zoom;

		// Cut out if out of grid bounds
		if(glx < 0 || gly < 0 || gfx >= g->w || gfy >= g->h)
			break;

		// Clip grid bounds
		if(gfx < 0) gfx = 0;
		if(gfy < 0) gfy = 0;
		if(glx > g->w) glx = g->w;
		if(gly > g->h) gly = g->h;

		uint16_t c = rgb16(32, 32, 32);

		// Get x,y
		int gx = gfx + rootimg->zoom - 1;
		int gy = gfy + rootimg->zoom - 1;

		// Draw vertical
		for(y = gfy; y < gly; y++)
		for(x = gx; x < glx; x += rootimg->zoom)
			*SCR16(sx + x, sy + y) = c;

		// Draw horizontal
		for(y = gy; y < gly; y += rootimg->zoom)
		for(x = gfx; x < glx; x++)
			*SCR16(sx + x, sy + y) = c;

	} while(0);

	// Draw grid
	if(tool_gw >= 1 && tool_gh >= 1) do
	{
		// Transform grid parameters
		int gx = (tool_gx - rootimg->zx) * rootimg->zoom - 1;
		int gy = (tool_gy - rootimg->zy) * rootimg->zoom - 1;
		int gw = tool_gw * rootimg->zoom;
		int gh = tool_gh * rootimg->zoom;

		// Transform grid bounds
		int gfx = (0 - rootimg->zx) * rootimg->zoom;
		int gfy = (0 - rootimg->zy) * rootimg->zoom;
		int glx = (rootimg->w - rootimg->zx) * rootimg->zoom;
		int gly = (rootimg->h - rootimg->zy) * rootimg->zoom;
		
		// Cut out if out of grid bounds
		if(glx < 0 || gly < 0 || gfx >= g->w || gfy >= g->h)
			break;

		// Clip grid bounds
		if(gfx < 0) gfx = 0;
		if(gfy < 0) gfy = 0;
		if(glx > g->w) glx = g->w;
		if(gly > g->h) gly = g->h;

		// Advance grid x,y
		if(gx < 0) gx = gw - (-gx % gw);
		if(gy < 0) gy = gh - (-gy % gh);

		uint16_t c = rgb16(85, 85, 85);

		// Draw vertical
		for(y = gfy; y < gly; y++)
		for(x = gx; x < glx; x += gw)
			*SCR16(sx + x, sy + y) = c;

		// Draw horizontal
		for(y = gy; y < gly; y += gh)
		for(x = gfx; x < glx; x++)
			*SCR16(sx + x, sy + y) = c;

	} while(0);

	// Draw corner #1
	if(tool_cx1 != -1)
	{
		x = (tool_cx1 - rootimg->zx) * rootimg->zoom - 1;
		y = (tool_cy1 - rootimg->zy) * rootimg->zoom - 1;
		if(tool_cx2 != -1 && tool_cx1 > tool_cx2) x += rootimg->zoom+1;
		if(tool_cy2 != -1 && tool_cy1 > tool_cy2) y += rootimg->zoom+1;

		if(x >= 0 && x < g->w)
		{
			draw_rect32(
				sx + x, sy,
				sx + x, sy + g->h-1,
				rgb32(0x00,0x80,0xFF));
		}

		if(y >= 0 && y < g->h)
		{
			draw_rect32(
				sx, /******/ sy + y,
				sx + g->w-1, sy + y,
				rgb32(0x00,0x80,0xFF));
		}
	}

	// Draw corner #2
	if(tool_cx2 != -1)
	{
		x = (tool_cx2 - rootimg->zx) * rootimg->zoom - 1;
		y = (tool_cy2 - rootimg->zy) * rootimg->zoom - 1;
		if(tool_cx2 >= tool_cx1) x += rootimg->zoom+1;
		if(tool_cy2 >= tool_cy1) y += rootimg->zoom+1;

		if(x >= 0 && x < g->w)
		{
			draw_rect32(
				sx + x, sy,
				sx + x, sy + g->h-1,
				rgb32(0xFF,0x80,0x00));
		}

		if(y >= 0 && y < g->h)
		{
			draw_rect32(
				sx, /******/ sy + y,
				sx + g->w-1, sy + y,
				rgb32(0xFF,0x80,0x00));
		}
	}

}

static void w_img_pack(widget_t *g, int w, int h)
{
	g->w = w;
	g->h = h;
}

static void w_img_mouse_button(widget_t *g, int mx, int my, int button, int state)
{
	int x = mx;
	int y = my;

	// Widget -> Image mapping
	x /= rootimg->zoom;
	y /= rootimg->zoom;
	x += rootimg->zx;
	y += rootimg->zy;
	
	// TODO: Deal with the issue where (w, h) % rootimg->zoom != 0

	if(tool_pasting && (button != 3 && button != 4))
	{
		if(state) return;
		if(key_mods_drag) return;
		if(button == 1) return;

		// Check button
		if(button == 0)
		{
			// Paste image
			int bx = x;
			int by = y;

			for(by = 0; by < clipimg->h; by++)
			for(bx = 0; bx < clipimg->w; bx++)
				if(bx+x >= 0 && bx+x < rootimg->w)
				if(by+y >= 0 && by+y < rootimg->h)
				if(tool_pasting == 1 || *IMG8(clipimg, bx, by) != tool_bgidx)
					*IMG8(rootimg, bx+x, by+y) = *IMG8(clipimg, bx, by);
		}

		// Drop pasting mode and return
		tool_pasting = 0;
		return;
	}

	if(!state) return;

	if(key_mods_drag)
	{
		if((key_mods_drag & KM_SHIFT) && !(key_mods_drag & ~KM_SHIFT))
		{
			// Set corner
			if(button == 0)
			{
				tool_cx1 = x;
				tool_cy1 = y;
			} else if(button == 2) {
				tool_cx2 = x;
				tool_cy2 = y;
			}

		}

		return;
	}

	if(button != 0 && button != 2)
	{
		// Calculate old zoom info
		int lzoom = rootimg->zoom;
		int lzx = rootimg->zx + mx/lzoom;
		int lzy = rootimg->zy + my/lzoom;

		// Check scroll wheels
		if(button == 4)
		{
			rootimg->zoom *= 2;
			if(rootimg->zoom > 64)
				rootimg->zoom = 64;

		} else if(button == 3) {
			rootimg->zoom /= 2;
			if(rootimg->zoom < 1)
				rootimg->zoom = 1;
		}

		// No scroll? Return.
		if(rootimg->zoom == lzoom)
			return;

		// Calculate new zoom info
		int nzoom = rootimg->zoom;
		int nzx = rootimg->zx + mx/nzoom;
		int nzy = rootimg->zy + my/nzoom;

		// Move camera
		rootimg->zx -= nzx - lzx;
		rootimg->zy -= nzy - lzy;

		return;
	}

	if(button == 0)
	{
		// Put a pixel somewhere
		if(x >= 0 && y >= 0 && x < rootimg->w && y < rootimg->h)
		{
			*IMG8(rootimg, x, y) = tool_palidx;
			rootimg->dirty = 1; // TODO: Several "dirty" flags
		}

	} else if(button == 2) {
		// Get a pixel from somewhere
		if(x >= 0 && y >= 0 && x < rootimg->w && y < rootimg->h)
			tool_palidx = *IMG8(rootimg, x, y);

	}
}

static void w_img_mouse_motion_shift(widget_t *g, int mx, int my, int dx, int dy, int button, int bail)
{
	int x = mx;
	int y = my;

	// Widget -> Image mapping
	x /= rootimg->zoom;
	y /= rootimg->zoom;
	x += rootimg->zx;
	y += rootimg->zy;
	
	// TODO: Deal with the issue where (w, h) % rootimg->zoom != 0

	// Set corner
	if(button == 0)
	{
		tool_cx1 = x;
		tool_cy1 = y;
	} else if(button == 2) {
		tool_cx2 = x;
		tool_cy2 = y;
	}


	// If bailing, warp the mouse
	// TODO: Make this trigger BEFORE the border
	// TODO REFACTOR THIS SO IT'S NOT DUPLICATED EVERYWHERE
	if(bail)
	{
		// Get image position
		int mdx = (mx >= g->w ? -1 : (mx < 0 ? 1 : 0)) * ((g->w>>1) / rootimg->zoom);
		int mdy = (my >= g->h ? -1 : (my < 0 ? 1 : 0)) * ((g->h>>1) / rootimg->zoom);

		// Move image
		rootimg->zx -= mdx;
		rootimg->zy -= mdy;

		// Fixes a bug with the left/top edges breaking
		if(mdx > 0) mdx++;
		if(mdy > 0) mdy++;

		// Move mouse
		SDL_WarpMouse(mouse_x + mdx*rootimg->zoom, mouse_y + mdy*rootimg->zoom);
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
		int mdx = (mx >= g->w ? -1 : (mx < 0 ? 1 : 0)) * ((g->w>>1) / rootimg->zoom);
		int mdy = (my >= g->h ? -1 : (my < 0 ? 1 : 0)) * ((g->h>>1) / rootimg->zoom);

		// Move image
		rootimg->zx -= mdx;
		rootimg->zy -= mdy;

		// Fixes a bug with the left/top edges breaking
		if(mdx > 0) mdx++;
		if(mdy > 0) mdy++;

		// Move mouse
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
		int mdx = (mx >= 7*(g->w>>3) ? -1 : (mx <= (g->w>>3) ? 1 : 0)) * (g->w>>1);
		int mdy = (my >= 7*(g->h>>3) ? -1 : (my <= (g->h>>3) ? 1 : 0)) * (g->h>>1);
		mdx /= rootimg->zoom;
		mdy /= rootimg->zoom;

		// TODO: Clamp to image bounds

		// Warp mouse
		SDL_WarpMouse(mouse_x + mdx*rootimg->zoom, mouse_y + mdy*rootimg->zoom);
	}

}

static void w_img_mouse_motion(widget_t *g, int mx, int my, int dx, int dy, int bail, int buttons)
{
	if(tool_pasting)
	{
		if(buttons & 2)
			buttons = 2;
		else
			return;
	}

	if(!key_mods_drag)
	{
		if((buttons & 1) != 0) return w_img_mouse_motion_lmb(g, mx, my, dx, dy, bail);
		if((buttons & 2) != 0) return w_img_mouse_motion_mmb(g, mx, my, dx, dy, bail);
	} else if((key_mods_drag & KM_SHIFT) && !(key_mods_drag & ~KM_SHIFT)) {
		if((buttons & 1) != 0) return w_img_mouse_motion_shift(g, mx, my, dx, dy, 0, bail);
		if((buttons & 4) != 0) return w_img_mouse_motion_shift(g, mx, my, dx, dy, 2, bail);
	}
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


/*
Copyright (c) 2014 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

const uint32_t dtab[4] = {
	0, 4,
	6, 2,
};

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
	g >>= 2;
	b >>= 3;

	return (r<<11) | (g<<5) | b;
}


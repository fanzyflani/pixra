/*
Copyright (c) 2014 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

uint16_t io_get2le(FILE *fp)
{
	int v0 = fgetc(fp);
	int v1 = fgetc(fp);

	return (v1<<8)|v0;
}

void io_put2le(int v, FILE *fp)
{
	int v0 = v&255;
	int v1 = (v>>8)&255;

	fputc(v0, fp);
	fputc(v1, fp);
}

void img_undirty(img_t *img)
{
	int i, j;

	// Check
	if(img->dirty == 0)
		return;

	// Dither palette
	for(i = 0; i < 4; i++)
	for(j = 0; j < 256; j++)
		img->dpal[i][j] = c32to16(img->pal[j], i&1, i>>1);

	// Clear
	img->dirty = 0;
}

void img_free(img_t *img)
{
	// Free some simple pointers
	if(img->data != NULL) free(img->data);
	if(img->ldata != NULL) free(img->ldata);
	if(img->fname != NULL) free(img->fname);

	// TODO: Undo stack

	// Free image
	free(img);
}

img_t *img_new(int w, int h)
{
	int x, y, i;

	img_t *img = malloc(sizeof(img_t));

	// Image stuff
	img->w = w;
	img->h = h;

	// Image state
	img->fname = NULL;
	img->dirty = 1;
	img->zoom = 2;
	img->zx = 0;
	img->zy = 0;
	img->undo_top = NULL;
	img->undo_bottom = NULL;
	img->undosz_total = 0;
	img->undosz_bottom = 0;

	// Image palette
#if 0
	for(i = 0; i < 256; i++)
	{
		int hi = i;
		int si = 0;
		int vi = 0;

		float r = sin(M_PI*hi*3.0/128.0);
		float g = sin(M_PI*hi*3.0/128.0 + M_PI*2.0/3.0);
		float b = sin(M_PI*hi*3.0/128.0 + M_PI*4.0/3.0);

		// TODO: Oscillate these
		float s = 1.0;
		float v = 1.0;

		r = v*((s*r + (1.0-s))*0.5 + 0.5);
		g = v*((s*g + (1.0-s))*0.5 + 0.5);
		b = v*((s*b + (1.0-s))*0.5 + 0.5);

		r = r*255.0 + 0.5;
		g = g*255.0 + 0.5;
		b = b*255.0 + 0.5;

		img->pal[i] = rgb32(r, g, b);
	}
#endif
	for(i = 0; i < 256; i++)
		img->pal[i] = rgb32(0xFF, 0, 0xFF);

	// EGA + grey 16 + websafe
	for(i = 0; i < 16; i++)
	{
		//
		int r = (i&4 ? 170 : 0);
		int g = (i&2 ? 170 : 0);
		int b = (i&1 ? 170 : 0);
		int intens = (i&8 ? 85 : 0); 

		if(i == 6) g = 85;

		img->pal[i] = rgb32(r|intens, g|intens, b|intens);
	}

	for(i = 0; i < 16; i++)
	{
		int c = (i+1)*0x0F;
		img->pal[i + 16] = rgb32(c, c, c);
	}

	for(i = 0; i < 216; i++)
	{
		int r = (i/36)*0x33;
		int g = ((i/6)%6)*0x33;
		int b = (i%6)*0x33;

		img->pal[i + 32] = rgb32(r, g, b);
	}

	// Image data
	img->data = malloc(w*h);
	img->ldata = malloc(w*h);

#if 0
	// TEST: XOR pattern
	for(y = 0; y < h; y++)
	for(x = 0; x < w; x++)
		*IMG8(img, x, y) = x^y;
#else
	// Clear image
	memset(img->data, 0, w*h);
#endif
	
	// Image return
	return img;
}

img_t *img_load_tga(const char *fname)
{
	FILE *fp;
	int x, y, i;
	int r, g, b;
	uint8_t t;

	// Open file
	fp = fopen(fname, "rb");
	if(fp == NULL)
	{
		perror("img_load_tga(fopen)");
		return NULL;
	}

	// Read TGA header
	uint8_t idlen = fgetc(fp);
	uint8_t cmaptyp = fgetc(fp);
	uint8_t datatyp = fgetc(fp);
	uint16_t cmapbeg = io_get2le(fp);
	uint16_t cmaplen = io_get2le(fp);
	uint8_t cmapbpp = fgetc(fp);
	int16_t ix = io_get2le(fp);
	int16_t iy = io_get2le(fp);
	uint16_t iw = io_get2le(fp);
	uint16_t ih = io_get2le(fp);
	uint8_t ibpp = fgetc(fp);
	uint8_t idesc = fgetc(fp);

	// Check if this image is supported
	if(datatyp != 1)
	{
		fprintf(stderr, "img_load_tga: only indexed tga images supported\n");
		fclose(fp);
		return NULL;
	}

	if(cmaptyp != 1)
	{
		fprintf(stderr, "img_load_tga: must contain a colour map of type 1\n");
		fclose(fp);
		return NULL;
	}

	if(ibpp != 8)
	{
		fprintf(stderr, "img_load_tga: only 8bpp image data supported\n");
		fclose(fp);
		return NULL;
	}

	if(cmapbpp != 24)
	{
		fprintf(stderr, "img_load_tga: only 24bpp colour maps supported\n");
		fclose(fp);
		return NULL;
	}

	if((idesc & ~0x20) != 0x00)
	{
		fprintf(stderr, "img_load_tga: given image descriptor flags not supported\n");
		fclose(fp);
		return NULL;
	}

	if(iw <= 0 || ih <= 0)
	{
		fprintf(stderr, "img_load_tga: invalid dimensions\n");
		fclose(fp);
		return NULL;
	}

	// Skip comment
	while(idlen-- > 0) fgetc(fp);

	// Create image
	img_t *img = img_new(iw, ih);
	img->fname = strdup(fname);

	// Load palette
	for(i = 0; i < cmaplen; i++)
	{
		b = fgetc(fp);
		g = fgetc(fp);
		r = fgetc(fp);

		img->pal[i] = rgb32(r, g, b);
	}

	// Clear remainder of palette
	for(; i < 256; i++)
		img->pal[i] = rgb32(0, 0, 0);
	
	// Load image
	fread(img->data, img->w, img->h, fp);

	// Flip if origin on bottom
	if((idesc & 0x20) == 0)
	{
		for(y = 0; y < (ih>>1); y++)
		for(x = 0; x < iw; x++)
		{
			t = *IMG8(img, x, y);
			*IMG8(img, x, y) = *IMG8(img, x, ih-1-y);
			*IMG8(img, x, ih-1-y) = t;
		}
	}

	// Close + return
	fclose(fp);
	return img;
}

int img_save_tga(const char *fname, img_t *img)
{
	FILE *fp;
	int x, y, i;

	// Check to see if we actually HAVE a filename
	if(fname == NULL)
	{
		fprintf(stderr, "img_save_tga: filename not set\n");
		return 1;
	}

	// Open file for writing
	fp = fopen(fname, "wb");
	if(fp == NULL)
	{
		perror("img_save_tga(fopen)");
		return 1;
	}

	// Header
	fputc(0, fp); // idlen
	fputc(1, fp); // cmaptyp
	fputc(1, fp); // datatyp
	io_put2le(0, fp); // cmapbeg
	io_put2le(256, fp); // cmaplen
	fputc(24, fp); // cmapbpp
	io_put2le(0, fp); // ix
	io_put2le(img->h, fp); // iy
	io_put2le(img->w, fp); // iw
	io_put2le(img->h, fp); // ih
	fputc(8, fp); // ibpp
	fputc(0x20, fp); // idesc

	// Palette
	for(i = 0; i < 256; i++)
	{
		uint32_t c = img->pal[i];

		fputc((c>>0)&0xFF, fp);
		fputc((c>>8)&0xFF, fp);
		fputc((c>>16)&0xFF, fp);
	}

	// Image data
	printf("writing data %i %i\n", img->w, img->h);
	fwrite(img->data, img->w, img->h, fp);

	// Close + return
	fclose(fp);
	printf("Image saved to \"%s\"\n", fname);
	return 0;
}

/*
Copyright (c) 2014 fanzyflani & contributors.
See LICENCE.txt for licensing information (TL;DR: MIT-style).
*/

#include "common.h"

const char *helptext[] = {
	"Keys (usable everywhere):",
	"* F1 = SHOW THIS HELP AT ANY TIME",
	"* Esc = Cancel operation",
	"* C = Toggle colour picker window",
	"* 1-9,0 = Change tool opacity",
	"* Ctrl-L = Load image",
	"* Ctrl-S = Save image",
	"* Ctrl-C = Copy image data",
	"* Ctrl-X = Cut image data, replacing with drawing colour",
	"* Ctrl-V = Paste image data transparently",
	"* Ctrl-B = Paste image data obliquely",
	"* Ctrl-R = Rectangle fill",
	"* Ctrl-F = Flood fill",
	"* Ctrl-Z = Undo",
	"* Ctrl-Shift-Z = Redo",
	"* Ctrl-G = Set grid according to box select",
	//"* (TODO) Ctrl-Shift-C = Copy palette data",
	//"* (TODO) Ctrl-Shift-V = Paste palette data",
	//"* (TODO) Ctrl-Shift-I = Interpolate palette data",

	"",
	"Image window:",
	"* Left Mouse Button: Draw with pencil",
	"* Right Mouse Button: Pick drawing colour from the image",
	"* Middle Mouse Button: Scroll the view",
	"* Scroll Wheel: Zoom in/out (\"pull\" to zoom in, \"push\" to zoom out)",
	"* Ctrl + Scroll Wheel: Change tool size",
	"* Shift + Left Mouse Button: Box select corner #1",
	"* Shift + Right Mouse Button: Box select corner #2",

	"",
	"Palette window:",
	"* Left Mouse Button: Pick drawing colour",
	"* Right Mouse Button: Pick transparent colour",
	//"* (TODO) Shift + Left Mouse Button: Range select end #1",
	//"* (TODO) Shift + Right Mouse Button: Range select end #2",

	"",
	"Colour picker window:",
	"* Left Mouse Button (on gradient): Change R/G/B component of drawing colour",

	NULL
};



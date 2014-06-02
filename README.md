pixra (or "xra" for short) is an image editor designed specifically for the Raspberry Pi. It is designed to edit 256-colour (8bpp) images with a 24bpp palette, and dithers it so it looks nice for the Raspi's native depth of 16bpp.

"pixra" is the Lojban word for "picture", and the x is that pholegmy sound that doesn't show up in English, but does show up in German and various Middle-Eastern languages.

Just in case you were wondering.

See USAGE.md for information on how to actually use pixra (namely, the shortcuts).
TIP: Printing it off is a good idea.

=== Most important TODO before a proper sane prerelease ===
We need a dialogue for loading / creating the initial image.
pixra is perfectly usable without it, but this will confuse people who aren't familiar with the commandline.
Note, if you're one of those people, you should learn how to use it from a commandline as it's nicer that way.

As a workaround, associate .tga files with pixra. This will mean that you won't be able to create new images, but you *will* be able to edit existing ones... as long as they're paletted images, that is.

=== Other TODOs ===
* The palette editing stuff is a bit lacking. I'd like to see these things:
  * Palette clipboard
  * Interpolate between colours
* UNDO STACK. I really really want to see this happen some time soon.
* "Tile assist" modes, mostly for making seamless tiles.
* Now that we have a font, I'd like to see it used.


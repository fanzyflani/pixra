pixra (or "xra" for short) is an image editor designed specifically for the Raspberry Pi. It is designed to edit 256-colour (8bpp) images with a 24bpp palette, and dithers it so it looks nice for the Raspi's native depth of 16bpp.

"pixra" is the Lojban word for "picture", and the x is that pholegmy sound that doesn't show up in English, but does show up in German and various Middle-Eastern languages.

Just in case you were wondering.

Usage:
- Keys (usable everywhere):
  - Esc = Cancel operation
  - C = Toggle colour picker window
  - Ctrl-L = Load image
  - Ctrl-S = Save image
  - Ctrl-C = Copy image data
  - Ctrl-X = Cut image data, replacing with drawing colour
  - Ctrl-V = Paste image data transparently
  - Ctrl-B = Paste image data obliquely
  - Ctrl-G = Set grid according to box select
  - Ctrl-R = Rectangle fill
  - (TODO) Ctrl-Shift-C = Copy palette data
  - (TODO) Ctrl-Shift-V = Paste palette data
  - (TODO) Ctrl-Shift-I = Interpolate palette data
- Image window:
  - Left Mouse Button: Draw with pencil
  - Right Mouse Button: Pick drawing colour from the image
  - Middle Mouse Button: Scroll the view
  - Scroll Wheel: Zoom in/out ("pull" to zoom in, "push" to zoom out)
  - Shift + Left Mouse Button: Box select corner #1
  - Shift + Right Mouse Button: Box select corner #2
- Palette window:
  - Left Mouse Button: Pick drawing colour
  - Right Mouse Button: Pick transparent colour
  - (TODO) Shift + Left Mouse Button: Range select inclusive end #1
  - (TODO) Shift + Right Mouse Button: Range select inclusive end #2


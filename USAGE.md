Command-line arguments:
* Windows:
  * Edit an existing image: pixra filename.tga
  * Create a new image: pixra filename.tga width height
* Linux/BSD (e.g. Raspberry Pi):
  * Edit an existing image: ./pixra filename.tga
  * Create a new image: ./pixra filename.tga width height

Keys (usable everywhere):
* Esc = Cancel operation
* C = Toggle colour picker window
* Ctrl-L = Load image
* Ctrl-S = Save image
* Ctrl-C = Copy image data
* Ctrl-X = Cut image data, replacing with drawing colour
* Ctrl-V = Paste image data transparently
* Ctrl-B = Paste image data obliquely
* Ctrl-Z = Undo
* Ctrl-Shift-Z = Redo
* Ctrl-G = Set grid according to box select
* Ctrl-R = Rectangle fill
* (TODO) Ctrl-Shift-C = Copy palette data
* (TODO) Ctrl-Shift-V = Paste palette data
* (TODO) Ctrl-Shift-I = Interpolate palette data

Image window:
* Left Mouse Button: Draw with pencil
* Right Mouse Button: Pick drawing colour from the image
* Middle Mouse Button: Scroll the view
* Scroll Wheel: Zoom in/out ("pull" to zoom in, "push" to zoom out)
* Shift + Left Mouse Button: Box select corner #1
* Shift + Right Mouse Button: Box select corner #2

Palette window:
* Left Mouse Button: Pick drawing colour
* Right Mouse Button: Pick transparent colour
* (TODO) Shift + Left Mouse Button: Range select end #1
* (TODO) Shift + Right Mouse Button: Range select end #2

Colour picker window:
* Left Mouse Button (on gradient): Change R/G/B component of drawing colour


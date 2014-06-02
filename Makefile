# TODO: BSD make compatibility
# (use gmake)
OBJDIR = build
SRCDIR = src
BINNAME = pixra

OBJS = \
	$(OBJDIR)/conv.o \
	$(OBJDIR)/draw.o \
	$(OBJDIR)/img.o \
	$(OBJDIR)/tool.o \
	$(OBJDIR)/widget.o \
	\
	$(OBJDIR)/main.o
	#

INCLUDES = src/common.h

CFLAGS = -Wall -Wextra -Wno-unused-parameter -O2 -g -Isrc -I/usr/local/include `sdl-config --cflags`
LDFLAGS = -O2 -g $(LIBS_DIRS) $(LIBS_SDL) -lm
LIBS_DIRS = -L/usr/local/lib
LIBS_SDL = `sdl-config --libs`

all: $(BINNAME)

clean:
	rm -r $(OBJDIR)

$(BINNAME): $(OBJDIR) $(OBJS)
	$(CC) -o $(BINNAME) $(LDFLAGS) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: all clean


# TODO: BSD make compatibility
# (use gmake)
OBJDIR = build
SRCDIR = src
BINNAME = xra

OBJS = \
	\
	$(OBJDIR)/main.o
	#

INCLUDES = src/common.h

CFLAGS = -g -Isrc -I/usr/local/include `sdl-config --cflags`
LDFLAGS = -g -L/usr/local/lib `sdl-config --libs` -lz -lm

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


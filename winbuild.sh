#!/bin/sh
gmake CC=mingw32-gcc OBJDIR=wbuild "CFLAGS=-Wall -Wextra -Wno-unused-parameter -O2 -g -Isrc -Iwinlibs -Iwinlibs/SDL" "LDFLAGS=-O2 -g -Lwinlibs -lmingw32 SDL.dll -lSDLmain" BINNAME=pixra.exe


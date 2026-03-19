# x68k/Makefile  --  X68000 / Human68k cross-build
#
# Requires m68k-elf-gcc (or XC cross-compiler) in PATH.
# Adjust CC, CFLAGS, and LDFLAGS for your toolchain.
#
# Usage:
#   make          -- build xfidx.x and xfind.x
#   make clean    -- remove build artefacts

CC      = m68k-elf-gcc
CFLAGS  = -Wall -O2 -fomit-frame-pointer \
          -D__human68k__ \
          -I../src

# Human68k runtime library path (adjust as needed)
# LDFLAGS = -L/path/to/hrt -lhrt
LDFLAGS =

SRC     = ../src

COMMON_OBJS = \
    $(SRC)/FSWALK.C \
    $(SRC)/INDEX.C  \
    $(SRC)/MATCH.C  \
    $(SRC)/ACTIONS.C \
    $(SRC)/CONFIG.C  \
    $(SRC)/UITUI.C

.PHONY: all clean

all: xfidx.x xfind.x

xfidx.x: $(SRC)/MXFIDX.C $(SRC)/FSWALK.C $(SRC)/INDEX.C $(SRC)/CONFIG.C
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

xfind.x: $(SRC)/MXFIND.C $(SRC)/INDEX.C $(SRC)/MATCH.C \
          $(SRC)/ACTIONS.C $(SRC)/CONFIG.C $(SRC)/UITUI.C
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f xfidx.x xfind.x *.o

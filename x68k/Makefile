# x68k/Makefile  --  X68000 / Human68k cross-build
#
# Toolchain: m68k-xelf-gcc (elf2x68k integrated toolchain)
# Install:   https://github.com/yunkya2/elf2x68k
#
# Usage:
#   make                          # m68k-xelf-gcc is in PATH
#   make TOOLCHAIN=/path/to/m68k-xelf  # specify toolchain root explicitly
#   make clean

# If TOOLCHAIN is set, prepend its bin/ to PATH so that the linker wrapper
# (ld.x) can find m68k-xelf-ld.bfd and elf2x68k.py by name.
# If m68k-xelf-gcc is already in PATH, leave TOOLCHAIN unset.
ifdef TOOLCHAIN
export PATH := $(TOOLCHAIN)/bin:$(PATH)
CROSS = $(TOOLCHAIN)/bin/m68k-xelf-
else
CROSS = m68k-xelf-
endif

CC = $(CROSS)gcc

# -m68000        : target MC68000 (base X68000 CPU)
# __m68k__ is predefined by the compiler; PLATFORM_X68K is set via PLATFORM.H
CFLAGS = -m68000 -Wall -O2 -fomit-frame-pointer \
         -I../src

# Basic C runtime is sufficient; no extra libs needed
LDFLAGS =

SRC = ../src

.PHONY: all clean

all: xfidx.x xfind.x

xfidx.x: $(SRC)/MXFIDX.C $(SRC)/FSWALK.C $(SRC)/INDEX.C $(SRC)/CONFIG.C
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

xfind.x: $(SRC)/MXFIND.C $(SRC)/INDEX.C $(SRC)/MATCH.C \
         $(SRC)/ACTIONS.C $(SRC)/CONFIG.C $(SRC)/UITUI.C
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f xfidx.x xfind.x *.o *.elf

#
# Libefi user-mode emulator makefile
# NOTE: this is *not* an EFI application, hence we don't use the libefi Makefile
#

LIBEFI := ..
CFLAGS := -I$(LIBEFI)/api -I$(LIBEFI)/api/amd64 -Isrc \
	-std=c11 -Wall -Wno-missing-braces -Wno-incompatible-pointer-types \
	-fshort-wchar -D_GNU_SOURCE -pthread $(shell pkg-config --cflags sdl2) -g
LDFLAGS := -pthread
LDLIBS := $(shell pkg-config --libs sdl2)

#
# Object files
#
OBJ := src/uemu.o src/peloader.o src/efiemu.o src/util.o \
	src/efisys/handle.o src/efisys/variable.o \
	src/protocol/console.o src/protocol/hii.o

.PHONY: all
all: uemu

uemu: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ) uemu

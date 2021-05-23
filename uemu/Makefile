# NOTE: this is *not* an EFI application, hence we don't use the libefi Makefile
LIBEFI := ..
CFLAGS := \
	-I$(LIBEFI)/api \
	-I$(LIBEFI)/api/amd64 \
	-std=c99 -Wall -Wno-missing-braces -fshort-wchar \
	-D_GNU_SOURCE -pthread -g \
	$(shell pkg-config --cflags sdl2)
LDFLAGS := -pthread

LDLIBS := $(shell pkg-config --libs sdl2)

#
# Object files
#
OBJ := src/uemu.o src/peloader.o src/efiemu.o src/gopemu.o src/util.o

.PHONY: all
all: uemu

uemu: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ) uemu

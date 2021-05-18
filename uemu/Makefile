# NOTE: this is *not* an EFI application, hence we don't use the libefi Makefile
LIBEFI := ..
CFLAGS := \
	-I$(LIBEFI)/api \
	-I$(LIBEFI)/api/amd64 \
	-std=c99 -Wall -D_GNU_SOURCE

.PHONY: all
all: uemu

uemu: uemu.o efiemu.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o uemu

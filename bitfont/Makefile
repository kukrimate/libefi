LIBEFI := ..
include ../tools/Makefile-$(ARCH).efi

# Application
APP := bitfont.efi

# Subsystem ID (EFI Application)
SUBSYSTEM := 10

.PHONY: all
all: $(APP)

$(APP): bitfont.o fbterm.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -f *.o $(APP)

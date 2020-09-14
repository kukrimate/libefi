include ../tools/Makefile.efi

# Application
APP := hdd/efi/boot/bootx64.efi

# Subsystem ID (EFI Application)
SUBSYSTEM := 10

all: $(APP)

$(APP): bitfont.o fbterm.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
clean:
	rm -f *.o $(APP)

include ../tools/Makefile.efi

# Revision identifier
CFLAGS += -DGIT_REV=L\"git.$(shell git rev-parse --short HEAD)\"

# EFI Application
SUBSYSTEM := 10

APP := hdd/efi/boot/bootx64.efi
OBJ := loadlin.o

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)

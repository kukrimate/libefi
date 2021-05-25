include $(LIBEFI)/tools/Makefile-$(ARCH).efi

# Application
APP := keyinfo$(EXT)

# Subsystem ID (EFI Application)
SUBSYSTEM := 10

.PHONY: all
all: $(APP)

$(APP): keyinfo.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *.efi

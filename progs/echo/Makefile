include $(LIBEFI)/tools/Makefile-$(ARCH).efi

# Application
APP := echo$(EXT)

# Subsystem ID (EFI Application)
SUBSYSTEM := 10

.PHONY: all
all: $(APP)

$(APP): echo.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *.efi

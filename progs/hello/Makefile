include $(LIBEFI)/tools/Makefile-$(ARCH).efi

# EFI Application
SUBSYSTEM := 10

APP := hello$(EXT)
OBJ := hello.o

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *.efi

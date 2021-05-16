# Default to x86_64
ARCH ?= x86_64
ifneq ($(ARCH), x86_64)
$(error loadlin only work on x86_64)
endif

# Include libefi
LIBEFI := ..
include ../tools/Makefile-$(ARCH).efi

# Revision identifier
CFLAGS += -DGIT_REV=L\"git.$(shell git rev-parse --short HEAD)\"

# EFI Application
SUBSYSTEM := 10

APP := loadlin$(EXT)
OBJ := loadlin.o

.PHONY: all
all: $(APP)

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *.efi

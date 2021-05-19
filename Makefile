# Default to x86_64
ARCH    ?= x86_64
LIBEFI  := $(shell pwd)

# Architecture independent examples
MODULES := efiutil progs/hello progs/bitfont progs/dumpvar

# x86_64 only loader example
ifeq ($(ARCH), x86_64)
MODULES += progs/loadlin
endif

.PHONY: all
all:
	for MOD in $(MODULES); do \
		$(MAKE) ARCH=$(ARCH) LIBEFI=$(LIBEFI) -C $$MOD/ all; \
	done


.PHONY: clean
clean:
	for MOD in $(MODULES); do \
		$(MAKE) ARCH=$(ARCH) LIBEFI=$(LIBEFI) -C $$MOD/ clean; \
	done

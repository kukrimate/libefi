# Default to x86_64
ARCH    ?= x86_64

# Architecture independent examples
MODULES := efiutil bitfont

# x86_64 only loader example
ifeq (ARCH, x86_64)
MODULES += loadlin
endif

.PHONY: all
all:
	for MOD in $(MODULES); do \
		$(MAKE) ARCH=$(ARCH) -C $$MOD/ all; \
	done


.PHONY: clean
clean:
	for MOD in $(MODULES); do \
		$(MAKE) ARCH=$(ARCH) -C $$MOD/ clean; \
	done

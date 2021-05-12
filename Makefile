ARCH    ?= x86_64
MODULES := efiutil bitfont loadlin

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

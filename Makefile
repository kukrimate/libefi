MODULES := efiutil bitfont loadlin

.PHONY: all
all:
	for MOD in $(MODULES); do \
		make -C $$MOD/ all; \
	done


.PHONY: clean
clean:
	for MOD in $(MODULES); do \
		make -C $$MOD/ clean; \
	done

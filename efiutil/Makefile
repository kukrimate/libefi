LIBEFI := ..
include ../tools/Makefile-$(ARCH).efi

.PHONY: all
all: libefiutil-$(ARCH).a

libefiutil-$(ARCH).a: efiutil.o string.o print.o
	$(AR) -cr $@ $^

.PHONY: clean
clean:
	rm -f *.o *.a

include ../tools/Makefile.efi

.PHONY: all clean

# All target
all: libefiutil.a

libefiutil.a: util.o
	$(AR) -cr $@ $^

# Clean built files
clean:
	rm -f *.o *.a

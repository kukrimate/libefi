include ../tools/Makefile.efi

OBJ := efiutil.o string.o print.o

.PHONY: all clean

# All target
all: libefiutil.a

libefiutil.a: $(OBJ)
	$(AR) -cr $@ $^

# Clean built files
clean:
	rm -f *.o *.a

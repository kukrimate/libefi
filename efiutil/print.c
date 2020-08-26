#include <efi.h>
#include <efiutil.h>
#include <stdarg.h>

static
void
putchar(efi_ch16 ch)
{
	efi_ch16 buffer[2];

	buffer[0] = ch;
	buffer[1] = 0;
	st->con_out->output_string(st->con_out, buffer);
}

static
void
puts(efi_ch16 *str)
{
	st->con_out->output_string(st->con_out, str);
}

static const efi_ch16 num_lookup[] = L"0123456789abcdef";

static
void
printptr(uintptr_t ptr)
{
	for (int i = sizeof(ptr) * 8; i; i -= 4) {
		putchar(num_lookup[(ptr >> (i - 4)) & 0xf]);
	}
}

static
void
printusig(unsigned n, unsigned b)
{
	char buf[10], *p = buf;

	do {
		*p++ = num_lookup[n % b];
	} while (n /= b);

	--p;
	for (; buf <= p; --p)
		putchar(*p);
}

static
void
printsig(int n, unsigned b)
{
	if (n < 0) {
		n *= -1;
		putchar('-');
	}
	printusig(n, b);
}

void
print(efi_ch16 *format, ...)
{
	va_list ap;

	va_start(ap, format);

	for (; *format; ++format) {
		if (*format == '%')
			switch (*++format) {
			case 'c':
				putchar((efi_ch16) va_arg(ap, int));
				break;
			case 's':
			case 'S':
				puts(va_arg(ap, efi_ch16 *));
				break;
			case 'p':
				printptr(va_arg(ap, uintptr_t));
				break;
			case 'x':
				printusig(va_arg(ap, unsigned), 16);
				break;
			case 'd':
				printsig(va_arg(ap, int), 10);
				break;
			case 'u':
				printusig(va_arg(ap, unsigned), 10);
				break;
			default:
				putchar('?');
				break;
			}
		else
			putchar(*format);
	}

	va_end(ap);
}

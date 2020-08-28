#include <efi.h>
#include <efiutil.h>
#include <stdarg.h>

static
void
efi_putchar(efi_ch16 ch)
{
	efi_ch16 buffer[2];

	buffer[0] = ch;
	buffer[1] = 0;
	st->con_out->output_string(st->con_out, buffer);
}

static
void
efi_puts(efi_ch16 *str)
{
	st->con_out->output_string(st->con_out, str);
}

static
efi_ch16 *digits = L"0123456789abcdef";

#define genprint_type(U, S) \
static \
void \
efi_print_##U(U num, int base) \
{ \
	efi_ch16 buf[20], *p; \
\
	p = buf + sizeof(buf); \
	*--p = 0; \
\
	do { \
		*--p = digits[num % base]; \
	} while (p >= buf && (num /= base)); \
\
	for (; *p; ++p) \
		efi_putchar(*p); \
} \
\
static \
void \
efi_print_##S(S num, int base) \
{ \
	if (num < 0) { \
		efi_putchar(L'-'); \
		num *= -1; \
	} \
	efi_print_##U(num, base); \
}

genprint_type(efi_u32, efi_i32)
genprint_type(efi_u64, efi_i64)

void
efi_print(efi_ch16 *fmt, ...)
{
	va_list va;
	_Bool wide;
	const char *p;

	va_start(va, fmt);
	for (; *fmt; ++fmt)
		switch (*fmt) {
		case L'%':
			wide = 0;
wide_redo:
			switch (*++fmt) {
			case L'l':
				wide = 1;
				goto wide_redo;
			case L'c':
				efi_putchar(va_arg(va, int));
				break;
			case L's':
				efi_puts(va_arg(va, efi_ch16 *));
				break;
			case L'p':
				efi_print_efi_u64(va_arg(va, efi_u64), 16);
				break;
			case L'x':
				if (wide)
					efi_print_efi_u64(va_arg(va, efi_u64), 16);
				else
					efi_print_efi_u32(va_arg(va, efi_u32), 16);
				break;
			case L'd':
				if (wide)
					efi_print_efi_i64(va_arg(va, efi_i64), 10);
				else
					efi_print_efi_i32(va_arg(va, efi_i32), 10);
				break;
			case L'%':
				efi_putchar(L'%');
				break;
			default:
				efi_putchar(L'?');
				break;
			}
			break;
		case '\r': /* Ignore CR */
			break;
		case L'\n': /* Write CRLF on LF */
			efi_puts(L"\r\n");
			break;
		default:
			efi_putchar(*fmt);
			break;
		}
	va_end(va);
}

/*
 * Formatted printing support
 */

#include <efi.h>
#include <efiutil.h>

void efi_puts(efi_ch16_t *str)
{
  efi_st->con_out->output_string(efi_st->con_out, str);
}

void efi_putchar(efi_ch16_t ch)
{
  efi_puts((efi_ch16_t[]) {ch, 0});
}

#define FLAG_LJUST    (1<<0)
#define FLAG_PLUS     (1<<1)
#define FLAG_SPACE    (1<<2)
#define FLAG_ZERO     (1<<3)
#define FLAG_ALTF     (1<<4)
#define FLAG_SIG      (1<<5)
#define FLAG_UPPER    (1<<6)

static int read_flags(efi_ch16_t **fmt)
{
  int flags = 0;
  efi_ch16_t *s = *fmt;

  for (;; ++s)
    switch (*s) {
    case L'-':
      flags |= FLAG_LJUST;
      break;
    case L'+':
      flags |= FLAG_PLUS;
      break;
    case L' ':
      flags |= FLAG_SPACE;
      break;
    case L'0':
      flags |= FLAG_ZERO;
      break;
    case L'#':
      flags |= FLAG_ALTF;
      break;
    default:
      *fmt = s;
      return flags;
    }
}

static size_t read_width(efi_ch16_t **fmt)
{
  size_t cnt = 0;
  efi_ch16_t *s = *fmt;

  for (;;)
    switch (*s) {
    case L'0' ... L'9':
      cnt = cnt * 10 + *s++ - L'0';
      break;
    default:
      *fmt = s;
      return cnt;
    }
}

#define LENGTH_NONE     (-1)
#define LENGTH_CHAR     0
#define LENGTH_SHORT    1
#define LENGTH_LONG     2
#define LENGTH_LLONG    3
#define LENGTH_SIZET    4
#define LENGTH_IMAXT    5
#define LENGTH_PDIFFT   6

static int read_length(efi_ch16_t **fmt)
{
    for (;;)
      switch (**fmt) {
      case L'h':
        if ((*fmt)[1] == L'h') {
          *fmt += 2;
          return LENGTH_CHAR;
        } else {
          *fmt += 1;
          return LENGTH_SHORT;
        }
      case L'l':
        if ((*fmt)[1] == L'l') {
          *fmt += 2;
          return LENGTH_LLONG;
        } else {
          *fmt += 1;
          return LENGTH_LONG;
        }
      case L'z':
        *fmt += 1;
        return LENGTH_SIZET;
      case L'j':
        *fmt += 1;
        return LENGTH_IMAXT;
      case L't':
        *fmt += 1;
        return LENGTH_PDIFFT;
      default:
        return LENGTH_NONE;
      }
}

static void print_num(int flags, size_t width, int base, uintmax_t num)
{
  if (flags & FLAG_SIG) {
    if ((intmax_t) num < 0) {
      num = -num;
      efi_putchar(L'-');
    } else if (flags & FLAG_PLUS) {
      efi_putchar(L'+');
    } else if (flags & FLAG_SPACE) {
      efi_putchar(L' ');
    }
  }

  if (flags & FLAG_ALTF) {
    if (base == 8) {
      efi_putchar(L'0');
    } else if (base == 16) {
      efi_putchar(L'0');
      if (flags & FLAG_UPPER) {
        efi_putchar(L'X');
      } else {
        efi_putchar(L'x');
      }
    }
  }

  efi_ch16_t buf[20], *p = buf;

  do {
    *p++ = ((flags & FLAG_UPPER) ? L"0123456789ABCDEF"
                                 : L"0123456789abcdef")[num % base];
    num /= base;
  } while (num > 0);

  size_t actual_width = p - buf;

  if (!(flags & FLAG_LJUST)) {
    for (; width > actual_width; --width) {
      efi_putchar(flags & FLAG_ZERO ? L'0' : L' ');
    }
  }

  do {
    efi_putchar(*--p);
  } while (p > buf);

  if (flags & FLAG_LJUST) {
    for (; width > actual_width; --width) {
      efi_putchar(L' ');
    }
  }
}

void efi_vprint(efi_ch16_t *fmt, va_list ap)
{
  for (; *fmt; ++fmt)
    switch (*fmt) {
    case L'%':
    {
      ++fmt;

      int flags = read_flags(&fmt);
      size_t width = read_width(&fmt);
      int length = read_length(&fmt);

      switch (*fmt) {
      case L'%':
        efi_putchar(L'%');
        break;
      case L'd':
      case L'i':
      case L'u':
      case L'x':
      case L'X':
      case L'o':
        {
          int base;
          if (*fmt == L'd' || *fmt == L'i') {
            flags |= FLAG_SIG;
            base = 10;
          } else if (*fmt == L'u') {
            base = 10;
          } else if (*fmt == L'x') {
            base = 16;
          } else if (*fmt == L'X') {
            flags |= FLAG_UPPER;
            base = 16;
          } else {
            base = 8;
          }

          uintmax_t val;
          switch (length) {
          case LENGTH_LONG:
            val = flags & FLAG_SIG ? va_arg(ap, long) : va_arg(ap, unsigned long);
            break;
          case LENGTH_LLONG:
            val = flags & FLAG_SIG ? va_arg(ap, long long) : va_arg(ap, unsigned long long);
            break;
          case LENGTH_SIZET:
            val = va_arg(ap, size_t);
            break;
          case LENGTH_IMAXT:
            val = va_arg(ap, uintmax_t);
            break;
          case LENGTH_PDIFFT:
            val = va_arg(ap, ptrdiff_t);
            break;
          default:
            val = flags & FLAG_SIG ? va_arg(ap, int) : va_arg(ap, unsigned int);
            break;
          }

          print_num(flags, width, base, val);
        }
        break;
      case L's':
        efi_puts(va_arg(ap, efi_ch16_t *));
        break;
      case L'c':
        efi_putchar(va_arg(ap, int));
        break;
      case L'p':
        print_num(FLAG_ALTF, 0, 16, (uintptr_t) va_arg(ap, void *));
        break;
      case L'g':
        {
          efi_guid_t *guid = va_arg(ap, efi_guid_t *);
          efi_print(L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            guid->data1, guid->data2, guid->data3,
            guid->data4[0], guid->data4[1], guid->data4[2],
            guid->data4[3], guid->data4[4], guid->data4[5],
            guid->data4[6], guid->data4[7]);
        }
        break;
      case L'G':
        {
          efi_guid_t *guid = va_arg(ap, efi_guid_t *);
          efi_print(L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            guid->data1, guid->data2, guid->data3,
            guid->data4[0], guid->data4[1], guid->data4[2],
            guid->data4[3], guid->data4[4], guid->data4[5],
            guid->data4[6], guid->data4[7]);
        }
        break;
      default:
        efi_putchar(L'?');
        break;
      }
      break;
    }
    case L'\r':
      break;
    case L'\n':
      efi_puts(L"\r\n");
      break;
    default:
      efi_putchar(*fmt);
      break;
    }
}

void efi_print(efi_ch16_t *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  efi_vprint(fmt, ap);
  va_end(ap);
}

/*
 * Formatted printing support
 *
 * The following format specifiers are supported:
 *  - %p:  pointers
 *  - %c:  efi_ch16_t
 *  - %g:  efi_guid_t
 *  - %s:  NUL-terminated efi_ch16_t string
 *  - %d:  32-bit signed,   decimal
 *  - %ld: 64-bit signed,   decimal
 *  - %u:  32-bit unsinged, decimal
 *  - %lu: 64-bit unsigned, decimal
 *  - %x:  32-bit unsinged, hex
 *  - %lx: 64-bit unsigned, hex
 */

#include <efi.h>
#include <efiutil.h>

// Print string
static void efi_puts(efi_ch16_t *str)
{
  efi_st->con_out->output_string(efi_st->con_out, str);
}

// Print character
static void efi_putchar(efi_ch16_t ch)
{
  efi_puts((efi_ch16_t[]) {ch, 0});
}

// Read the number of padding characters from the format string
static efi_size_t read_padcnt(efi_ch16_t **fmt)
{
  efi_size_t cnt = 0;
  for (;; ++*fmt)
    switch (**fmt) {
    case L'0' ... L'9':
      cnt = cnt * 10 + **fmt - L'0';
      break;
    default:
      return cnt;
    }
}

// Print formatted number
static void print_num(unsigned long long num, efi_size_t base, efi_size_t pad, _Bool sig)
{
  _Bool neg = 0;
  efi_ch16_t buf[20], *p = buf;

  // Check for negative sign if caller says signed type
  if (sig && (long long) num < 0) {
    num *= -1ULL;
    neg = 1;
  }

  // Convert digits
  do
    *p++ = L"0123456789abcdef"[num % base];
  while ((num /= base));

  // Print sign (if any)
  if (neg)
    efi_putchar(L'-');
  // Print padding (if any)
  for (efi_size_t i = p - buf + neg; i < pad; ++i)
    efi_putchar(L'0');
  // Print digits
  do
    efi_putchar(*--p);
  while (p > buf);
}

void efi_print(efi_ch16_t *fmt, ...)
{
  va_list va;
  efi_size_t padcnt;
  efi_guid_t *guid;

  va_start(va, fmt);
  for (; *fmt; ++fmt)
    switch (*fmt) {
    case L'%':
      switch (*++fmt) {
      case L'c':
        efi_putchar(va_arg(va, int));
        break;
      case L's':
        efi_puts(va_arg(va, efi_ch16_t *));
        break;
      case L'p':
        efi_puts(L"0x");
        print_num((efi_size_t) va_arg(va, void *), /*base=*/16, /*pad=*/0, /*sign=*/0);
        break;
      case L'd': // 32-bit
        print_num(va_arg(va, efi_i32_t), /*base=*/10, /*pad=*/0, /*sign=*/1);
        break;
      case L'u':
        print_num(va_arg(va, efi_u32_t), /*base=*/10, /*pad=*/0, /*sign=*/0);
        break;
      case L'x':
        print_num(va_arg(va, efi_u32_t), /*base=*/16, /*pad=*/0, /*sign=*/0);
        break;
      case L'0': // 0-padded
        ++fmt;
        padcnt = read_padcnt(&fmt);
        switch (*fmt) {
        case L'd': // 32-bit
          print_num(va_arg(va, efi_i32_t), /*base=*/10, /*pad=*/padcnt, /*sign=*/1);
          break;
        case L'u':
          print_num(va_arg(va, efi_u32_t), /*base=*/10, /*pad=*/padcnt, /*sign=*/0);
          break;
        case L'x':
          print_num(va_arg(va, efi_u32_t), /*base=*/16, /*pad=*/padcnt, /*sign=*/0);
          break;
        case L'l': // 64-bit
          switch (*++fmt) {
          case L'd':
            print_num(va_arg(va, efi_i64_t), /*base=*/10, /*pad=*/padcnt, /*sign=*/1);
            break;
          case L'u':
            print_num(va_arg(va, efi_u64_t), /*base=*/10, /*pad=*/padcnt, /*sign=*/0);
            break;
          case L'x':
            print_num(va_arg(va, efi_u64_t), /*base=*/16, /*pad=*/padcnt, /*sign=*/0);
            break;
          default:
            goto unknown;
          }
          break;
        default:
          goto unknown;
        }
        break;
      case L'l': // 64-bit
        switch (*++fmt) {
        case L'd':
          print_num(va_arg(va, efi_i64_t), /*base=*/10, /*pad=*/0, /*sign=*/1);
          break;
        case L'u':
          print_num(va_arg(va, efi_u64_t), /*base=*/10, /*pad=*/0, /*sign=*/0);
          break;
        case L'x':
          print_num(va_arg(va, efi_u64_t), /*base=*/16, /*pad=*/0, /*sign=*/0);
          break;
        default:
          goto unknown;
        }
        break;
      case L'g':
        guid = va_arg(va, efi_guid_t *);
        efi_print(L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
          guid->data1, guid->data2, guid->data3,
          guid->data4[0], guid->data4[1], guid->data4[2],
          guid->data4[3], guid->data4[4], guid->data4[5],
          guid->data4[6], guid->data4[7]);
        break;
      case L'%':
        efi_putchar(L'%');
        break;
      default:
      unknown:
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

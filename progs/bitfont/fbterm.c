/*
 * Framebuffer terminal
 */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "font.h"
#include "fbterm.h"

void fb_plot(struct fbinfo *fb,
	 uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b)
{
	unsigned char *pixel =
		(void *) fb->base + x * fb->pixelwidth + y * fb->linewidth;
	pixel[fb->red_idx] = r;
	pixel[fb->grn_idx] = g;
	pixel[fb->blu_idx] = b;
}

void fb_clear(struct fbinfo *fb, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t x, y;

	for (x = 0; x < fb->x_limit; ++x)
		for (y = 0; y < fb->y_limit; ++y)
			fb_plot(fb, x, y, r, g, b);
}

static struct font *fb_font = &font_8x16;
static uint32_t fb_row = 0, fb_col = 0;

void fb_putchar(struct fbinfo *fb, char ch)
{
	unsigned char *glyph;
	uint8_t line, bit;

	if (fb_row >= fb->y_limit / fb_font->lines) {
		/*
		 * Scroll one line
		 * TODO: replace slow memmove with optimized assembly
		 */
		memmove(
			(void *) fb->base,
			(void *) fb->base + fb->linewidth * fb_font->lines,
			fb->linewidth * (fb->y_limit - fb_font->lines));
		--fb_row;
	}

	switch (ch) {
	case '\r':
	case '\n':
		fb_col = 0;
		++fb_row;
		break;
	default:
		glyph = fb_font->glyphs + fb_font->lines * ch;
		for (line = 0; line < fb_font->lines; ++line)
			for (bit = 0; bit < fb_font->bits; ++bit)
				if (glyph[line] & (0x80 >> bit))
					fb_plot(fb,
						fb_col * fb_font->bits + bit,
						fb_row * fb_font->lines + line,
						200, 200, 200);
				else
					fb_plot(fb,
						fb_col * fb_font->bits + bit,
						fb_row * fb_font->lines + line,
						0, 0, 0);
		if (++fb_col >= fb->x_limit / fb_font->bits) {
			fb_col = 0;
			++fb_row;
		}
		break;
	}
}

static const char *digits = "0123456789abcdef";

#define genprint(U, S) \
static void fb_print_##U(struct fbinfo *fb, U num, int base) \
{ \
	char buf[20], *p; \
\
	p = buf + sizeof(buf); \
	*--p = 0; \
\
	do { \
		*--p = digits[num % base]; \
	} while (p >= buf && (num /= base)); \
\
	for (; *p; ++p) \
		fb_putchar(fb, *p); \
} \
\
static void fb_print_##S(struct fbinfo *fb, S num, int base) \
{ \
	if (num < 0) { \
		fb_putchar(fb, '-'); \
		num *= -1; \
	} \
	fb_print_##U(fb, num, base); \
}

genprint(uint32_t, int32_t)
genprint(uint64_t, int64_t)

void fb_print(struct fbinfo *fb, const char *fmt, ...)
{
	va_list va;
	_Bool wide;
	const char *p, buf[20];

	va_start(va, fmt);
	for (; *fmt; ++fmt)
		switch (*fmt) {
		case '%':
			wide = 0;
wide_redo:
			switch (*++fmt) {
			case 'l':
				wide = 1;
				goto wide_redo;
			case 'c':
				fb_putchar(fb, va_arg(va, int));
				break;
			case 's':
				for (p = va_arg(va, const char *); *p; ++p)
					fb_putchar(fb, *p);
				break;
			case 'p':
				fb_print_uint64_t(fb, va_arg(va, uint64_t), 16);
				break;
			case 'x':
				if (wide)
					fb_print_uint64_t(fb, va_arg(va, uint64_t), 16);
				else
					fb_print_uint32_t(fb, va_arg(va, uint32_t), 16);
				break;
			case 'd':
				if (wide)
					fb_print_int64_t(fb, va_arg(va, int64_t), 10);
				else
					fb_print_int32_t(fb, va_arg(va, int32_t), 10);
				break;
			case '%':
				fb_putchar(fb, '%');
				break;
			default:
				fb_putchar(fb, '?');
				break;
			}
			break;
		default:
			fb_putchar(fb, *fmt);
			break;
		}
	va_end(va);
}

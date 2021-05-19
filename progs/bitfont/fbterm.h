#ifndef FBTERM_H
#define FBTERM_H

struct fbinfo {
	/* Physical address of the framebuffer */
	uint64_t base;
	/* Horizontal and vertical resolution */
	uint32_t x_limit, y_limit;
	/* Length of one line in bytes */
	uint32_t linewidth;
	/* Length of one pixel in bytes */
	uint8_t pixelwidth;
	/* Offset for each color */
	uint8_t red_idx, grn_idx, blu_idx;
};

void
fb_plot(struct fbinfo *fb,
	 uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);

void
fb_clear(struct fbinfo *fb, uint8_t r, uint8_t g, uint8_t b);

void
fb_putchar(struct fbinfo *fb, char ch);

void
fb_print(struct fbinfo *fb, const char *fmt, ...);

#endif

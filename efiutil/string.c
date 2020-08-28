/*
 * Standard string functions
 */
#include <stddef.h>

void *
memset(void *s, int c, size_t n)
{
	char *p;

	for (p = s; n; --n)
		*p++ = c;

	return s;
}

void *
memmove(void *dest, void *src, size_t n)
{
	char *d, *s;

	d = dest;
	s = src;

	if (s < d) {
		d += n;
		s += n;
		while (n--)
			*--d = *--s;
	} else {
		while (n--)
			*d++ = *s++;
	}

	return dest;
}

void *
memcpy(void *dest, void *src, size_t n)
{
	char *d, *s;

	d = dest;
	s = src;

	while (n--)
		*d++ = *s++;

	return dest;
}

size_t
strlen(const char *s)
{
	const char *p;

	for (p = s; *p; ++p)
		;

	return p - s;
}

#include <syscall.h>
#include <string.h>
#include <varg.h>

/*
 * Special format:
 *  "%*s", number, string : print number-width string
 */
int vsprintf(char *buf, char *fmt, va_list args)
{
	char *dst = buf;
	unsigned int u;		/* value for %u/%x */
	int d;			/* value for %d */
	char *s;		/* value for %s */
	void *p;		/* value for %p */
	int zero;		/* completion with 0 */
	int width;		/* width field */
	int left;		/* left aligned */
	int len;		/* parsed string length */
	int swidth;		/* special format: %*s */

	while (*fmt) {
		if (*fmt != '%') {
			*dst++ = *fmt++;
			continue;
		}
		fmt++;		/* skip '%' */
		zero = 0;
		width = 0;
		swidth = 0;
		left = 0;
		/* handle alignment direction */
		while (*fmt == '+' || *fmt == '-') {
			left = (*fmt == '-');
			fmt++;
		}
		/* handle width */
		while (*fmt == '0') {	/* 0 completion */
			zero = 1;
			fmt++;
		}
		if (*fmt >= '1' && *fmt <= '9')
			width = strtoi(fmt, &fmt, 10);
		/* handle undefinite width of string */
		if (*fmt == '*') {
			swidth = va_arg(args, int);
			width = 0;	/* zero standard width */
			fmt++;
		}
		/* handle format char */
		switch (*fmt) {
		case '%':
			*dst = '%';
			len = 1;
			break;
		case 'c':
			*dst = va_arg(args, char);
			len = 1;
			break;
		case 'd':
			d = va_arg(args, int);
			len = itostr(d, dst, 10);
			break;
		case 'u':
			u = va_arg(args, unsigned int);
			len = utostr(u, dst, 10);
			break;
		case 'x':
			u = va_arg(args, unsigned int);
			len = utostr(u, dst, 16);
			break;
		case 'p':
			p = va_arg(args, void *);
			*dst++ = '0';
			*dst++ = 'x';
			len = utostr((unsigned int)p, dst, 16);
			break;
		case 's':
			s = va_arg(args, char *);
			if (swidth > 0)
				len = strnncpy(dst, s, swidth);
			else
				len = strcpy(dst, s);
			break;
		}
		if (width > len) {
			if (left) {
				memset(dst + len, ' ',  width - len);
			} else {
				memmove(dst + width - len, dst, len);
				memset(dst, zero ? '0' : ' ', width - len);
			}
			len = width;
		}
		fmt++;
		dst += len;
	}
	*dst = '\0';
	return (dst - buf);
}

static char ustrbuf[0x1000];

int printf(char *format, ...)
{
	va_list args;
	int i;
	va_start(args, format);
	i = vsprintf(ustrbuf, format, args);
	va_end(args);
	return usys_puts(ustrbuf);
}

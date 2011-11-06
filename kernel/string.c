#include <types.h>
#include <mm.h>

int searchzerobit(void *src, int bits, int set)
{
	unsigned int bit32;
	int i, k, off;
	off = bits & 31;
	bits /= 32;
	for (i = 0; i < bits; i++) {
		bit32 = *((unsigned int *)src);
		for (k = 0; k < 32; k++) {
			if (!(bit32 & 1))
				goto found;
			bit32 >>= 1;
		}
		src += 4;
	}
	bit32 = *((unsigned int *)src);
	for (k = 0; k < off; k++) {
		if (!(bit32 & 1))
			goto found;
		bit32 >>= 1;
	}
	return -1;
found:
	if (set)
		((unsigned char *)src)[k >> 3] |= (1 << (k & 7));
	return i * 32 + k;
}

int testbit(void *src, int bits)
{
	u32 bit32;
	bit32 = ((u32 *)src)[bits / 32];
	return bit32 & (1 << (bits & 31));
}

int setbit(void *src, int bits)
{
	u8 *bit8, r;
	bit8 = (u8 *)src + (bits / 8);
	r = *bit8 & 1 << (bits & 7);
	*bit8 = (*bit8) | (1 << (bits & 7));
	return !!r;
}

int clearbit(void *src, int bits)
{
	u8 *bit8, r;
	bit8 = (u8 *)src + (bits / 8);
	r = (*bit8) & (1 << (bits & 7));
	*bit8 = (*bit8) & ~(1 << (bits & 7));
	return !!r;
}

void memmove(void *dst, void *src, int n)
{
	/* safe copy */
	if (src + n > dst && src <= dst) {
		while (--n >= 0)
			*((u8 *)dst + n) = *((u8 *)src + n);
	} else {
		int i;
		for (i = 0; i < n; i++)
			*((u8 *)dst + i) = *((u8 *)src + i);
	}
}

void memcpy(void *dst, void *src, int n)
{
	int i;
	for (i = 0; i < n; i++)
		*((u8 *)dst + i) = *((u8 *)src + i);
}

void memset(void *dst, int c, int n)
{
	int i;
	for (i = 0; i < n; i++)
		*((u8 *)dst + i) = c;
}

void wmemset(void *dst, int c, int n)
{
	int i;
	for (i = 0; i < n; i++)
		*((u16 *)dst + i) = c;
}

int strlen(char *str)
{
	int len = 0;
	while (*str) {
		len++;
		str++;
	}
	return len;
}

int strcpy(char *dst, char *src)
{
	char *ret = dst;
	while (*src)
		*dst++ = *src++;
	*dst = '\0';
	return dst - ret;
}

/* return number of copied bytes */
int strncpy(char *dst, char *src, int n)
{
	int i = 0;
	while (*src && n > 0) {
		*dst++ = *src++;
		n--;
		i++;
	}
	while (n > 0) {
		*dst++ = '\0';
		n--;
	}
	return i;
}

int strnncpy(char *dst, char *src, int n)
{
	int i = 0;
	while (*src && n > 0) {
		*dst++ = *src++;
		n--;
		i++;
	}
	/* 
	 * Dont pad the remainder of dest with null
	 * if the length of src is less than n 
	 */
	return i;
}

int strcmp(char *s1, char *s2)
{
	while (*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

int strncmp(char *s1, char *s2, unsigned int n)
{
	while (n > 0 && *s1 && *s1 == *s2) {
		s1++;
		s2++;
		n--;
	}
	return (n > 0) ? (*s1 - *s2) : 0;
}

#define BASEMAX ('z' - 'a' + 11)
#define BASESTR "0123456789abcdefghijklmnopqrstuvwxyz"

/* return the lenth of number string */
int utostr(unsigned int n, char *nptr, int base)
{
	int i;
	char *p = nptr;
	/* base check */
	if (base < 2 || base > BASEMAX)
		return 0;
	/* fast process */
	if (n == 0) {
		nptr[0] = '0';
		nptr[1] = '\0';
		return 1;
	}
	/* transform number to string */
	while (n) {
		i = n % base;
		n /= base;
		*p++ = BASESTR[i];
	}
	/* save number string lenth */
	n = p - nptr;
	*p-- = '\0';
	/* reverse number string */
	while (nptr < p) {
		i = *p;
		*p = *nptr;
		*nptr = i;
		nptr++;
		p--;
	}
	return n;
}

/* return the lenth of number string */
int itostr(int n, char *nptr, int base)
{
	int i;
	char *p = nptr;
	/* base check */
	if (base < 2 || base > BASEMAX)
		return 0;
	/* fast process */
	if (n == 0) {
		nptr[0] = '0';
		nptr[1] = '\0';
		return 1;
	}
	/* If n is mininum of integer,  */
	if (n < 0) {
		/*
		 * Integer overflow:
		 * Cant transform mininum integer to postive number
		 */
		if (n == MIN_INT)
			return 0;
		n = -n;
		*p++ = '-';
	}
	/* transform number to string */
	while (n) {
		i = n % base;
		n /= base;
		*p++ = BASESTR[i];
	}
	/* save number string lenth */
	n = p - nptr;
	*p-- = '\0';
	if (*nptr == '-')
		nptr++;
	/* reverse number string */
	while (nptr < p) {
		i = *p;
		*p = *nptr;
		*nptr = i;
		nptr++;
		p--;
	}
	return n;
}

/*
 * transform string to integer
 * only support base: 16
 */
int strtoi(char *nptr, char **endptr, int base)
{
	int num = 0;
	int n;

	if (base < 2 || base > BASEMAX)
		return 0;

	while (*nptr) {
		n = *nptr;
		if (n >= '0' && n <= '9')
			n -= '0';
		else if (n >= 'A' && n <= 'Z')
			n -= ('A' - 10);
		else if (n >= 'a' && n <= 'z')
			n -= ('a' - 10);
		else
			n = base;
		if (n >= base)
			break;
		num = num * base + n;
		nptr++;
	}
	if (endptr)
		*endptr = nptr;
	return num;
}

int log2(int n)
{
	int i = -1;
	while (n > 0) {
		n >>= 1;
		i++;
	}
	return i;
}

int log2up(int n)
{
	int i = 0;
	if (n <= 0)
		return -1;
	n--;
	while (n > 0) {
		n >>= 1;
		i++;
	}
	return i;
}

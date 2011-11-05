#include <ulib.h>
#include <ctype.h>

int get_line(char *buf, int bufsz)
{
	int len;
	len = gets(buf, bufsz - 1);
	buf[len] = '\0';
	if (len > 0 && buf[len - 1] == '\n') {
		buf[len - 1] = '\0';
		len--;
	}
	return len;
}

static char *get_arg(char **pp)
{
	char *ret, *p;
	ret = NULL;
	p = *pp;
	/* skip white space and fill \0 */
	while (isblank(*p)) {
		*p = '\0';
		p++;
	}
	if (*p == '\0')
		goto out;
	ret = p;
	/* skip normal character */
	while (*p && !isblank(*p))
		p++;
out:
	*pp = p;
	return ret;
}

int parse_line(char *line, int len, char **argv)
{
	int argc;
	char *p, *pp;
	/* assert len >= 0 */
	if (len == 0)
		return 0;

	p = pp = line;
	argc = 0;
	while ((p = get_arg(&pp)) != NULL)
		argv[argc++] = p;
	return argc;
}


#include <ulib.h>

static int linelen = 0;
static char *dir = ".";
struct dir_stat ds[16];
struct file_stat fs;

/* This program only works for minixfs. */
#define MINIX_DENTRY_SIZE 16

static void display_dir(int n)
{
	int k;
	for (k = 0; k < n; k++) {
		if (linelen + ds[k].len + 1 >= 80) {
			printf("\n");
			linelen = 0;
		}
		linelen += ds[k].len + 1;
		printf("%*s ", ds[k].len, ds[k].name);
	}
}

static int list_dir(int fd)
{
	int r, i, n;
	n = fs.size / MINIX_DENTRY_SIZE;
	/* get dir stat entry and display it */
	for (i = 0; i < n; i += 16) {
		r = fgetdir(fd, i, 16, ds);
		if (r > 0)
			display_dir(r);
		else
			break;
	}
	if (r < 0)
		printf("fgetdir error\n");
	else if (linelen > 0)
		printf("\n");
	return r;
}

int main(int argc, char **argv)
{
	int fd, r;
	if (argc > 2) {
		printf("Usage: ls [file]\n");
		return -1;
	} else if (argc == 2) {
		dir = argv[1];
	}
	/* open dir */
	fd = open(dir, 0);
	if (fd < 0) {
		printf("Cannot open %s\n", dir);
		return -1;
	}
	/* get stat */
	if (fstat(fd, &fs) == 0) {
		if (S_ISDIR(fs.mode))
			r = list_dir(fd);	/* list dir */
		else
			printf("%s is not dir\n", dir);
	} else {
		printf("Cannot get stat of file %s\n", dir);
	}
	/* close dir */
	close(fd);
	return r;
}

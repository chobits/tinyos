#include <ulib.h>

int main(int argc, char **argv)
{
	char buf[512];
	struct file_stat fs;
	int fd, n, r = -1;

	/* parse arguments */
	if (argc > 2) {
		printf("Usage: cat [file]\n");
		return -1;
	}

	/* cat the keyboard input */
	if (argc == 1) {
		while ((n = gets(buf, 512)) > 0)
			printf("%*s", n, buf);
		return 0;
	}

	/* open file */
	fd = open(argv[1], 0);
	if (fd < 0) {
		printf("cat: open file %s error\n", argv[1]);
		return -1;
	}
	/* get stat */
	if (fstat(fd, &fs) == 0) {
		if (S_ISREG(fs.mode)) {
			/* cat it! */
			while ((n = read(fd, buf, 512)) > 0)
				printf("%*s", n, buf);
			r = 0;
		} else {
			printf("%s is not regular file\n", argv[1]);
		}
	} else {
		printf("Cannot get stat of file %s\n", argv[1]);
	}
	/* safe exit */
	close(fd);
	return r;
}


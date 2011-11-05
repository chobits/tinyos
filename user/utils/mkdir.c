#include <ulib.h>

int main(int argc, char **argv)
{
	int fd, i;
	if (argc == 1) {
		printf("Usage: mkdir dirs...\n");
		return -1;
	}
	for (i = 1; i < argc; i++) {
		/* make directory */
		fd = mkdir(argv[i], 0);
		if (fd < 0)
			printf("cannot make directory: %s\n", argv[i]);
		else
			close(fd);
	}
	return 0;
}

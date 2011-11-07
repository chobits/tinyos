#include <ulib.h>

int main(int argc, char **argv)
{
	int i;
	if (argc == 1) {
		printf("Usage: rm files...\n");
		return -1;
	}
	for (i = 1; i < argc; i++) {
		/* make directory */
		if (rm(argv[i]) < 0)
			printf("cannot remove files: %s\n", argv[i]);
	}
	return 0;
}

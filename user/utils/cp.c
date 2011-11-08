#include <ulib.h>

char buf[1024];

int copy(int src, int dst)
{
	int size;
	while ((size = read(src, buf, 1024)) > 0) {
		if (write(dst, buf, size) != size) {
			printf("write error\n");
			size = -1;
			break;
		}
	}
	return size;
}

int main(int argc, char **argv)
{
	int src, dst;
	int r = -1;
	if (argc != 3) {
		printf("Usage: cp srcfile dstfile\n");
		return -1;
	}

	src = open(argv[1], 0);
	if (src < 0)
		goto out;
	dst = open(argv[2], O_CREATE | O_TRUNCATE);
	if (dst < 0)
		goto out_close;
	r = copy(src, dst);
	close(dst);
out_close:
	close(src);
out:
	return r;
}


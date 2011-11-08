#include <ulib.h>

char buf[1024];
char *sfile, *dfile;

int check(int src, int dst)
{
	struct file_stat ss, ds;
	if (fstat(src, &ss) < 0) {
		printf("Cannot stat %s\n", sfile);
		return -1;
	}
	if (fstat(dst, &ds) < 0) {
		printf("Cannot stat %s\n", dfile);
		return -1;
	}
	if (ss.inode == ds.inode) {
		printf("%s and %s are the same file\n", sfile, dfile);
		return -1;
	}
	if (!S_ISREG(ss.mode)) {
		printf("%s is not regular file\n", sfile);
		return -1;
	}
	if (!S_ISREG(ds.mode)) {
		printf("%s is not regular file\n", dfile);
		return -1;
	}
	/* truncate it */
	if (truncate(dst) < 0) {
		printf("%s cannot be truncated\n", dfile);
		return -1;
	}
	return 0;
}

int copy(int src, int dst)
{
	int size;
	while ((size = read(src, buf, 1024)) > 0) {
		if (write(dst, buf, size) != size) {
			printf("Write %s error\n", dfile);
			return -1;
		}
	}
	if (size < 0)
		printf("Read %s error\n", sfile);
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

	sfile = argv[1];
	dfile = argv[2];
	/* open */
	src = open(sfile, 0);
	if (src < 0)
		goto out;
	dst = open(dfile, O_CREATE);
	if (dst < 0)
		goto out_close;
	/* check the files */
	r = check(src, dst);
	if (!r) {
		/* copy! */
		r = copy(src, dst);
	}
	/* close */
	close(dst);
out_close:
	close(src);
out:
	return r;
}


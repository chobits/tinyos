#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

static char *boot, *kernel;
static int bfd, kfd;
static int ksize, fsoff;
static unsigned int sectors;

static void die(char *str)
{
	if (errno)
		perror(str);
	else
		fprintf(stderr, "%s\n", str);
	exit(EXIT_FAILURE);
}

static void usage(void)
{
	fprintf(stderr, "build bootimage kernelimage sectors\n");
	exit(EXIT_FAILURE);
}

static void parse_args(int argc, char **argv)
{
	if (argc != 4)
		usage();
	boot = argv[1];
	kernel = argv[2];
	sectors = atoll(argv[3]);
	/* must be larger than 2MB */
	if (sectors < 4096)
		die("sectors size is too small");
}

void fwriteoffset(unsigned int offset)
{
	FILE *fp = fopen(".offset", "w");
	if (!fp)
		die("Cannot create .offset file");
	fprintf(fp, "%u", offset);
	fclose(fp);
}

static void open_image(void)
{
	bfd = open(boot, O_WRONLY);
	if (bfd == -1)
		die("open boot image");
	kfd = open(kernel, O_RDONLY);
	if (kfd == -1)
		die("open kernel image");
}

static void build_image(void)
{
	/* write sectors of kernel image into offset 508(2 bytes) of boot.bin*/
	ksize = lseek(kfd, 0, SEEK_END);
	if (ksize < 0)
		die("lseek kernel image");
	ksize = (ksize + 511) / 512;
	if (ksize > 0xffff)
		die("kernel image size is too large!");
	if (lseek(bfd, 508, SEEK_SET) != 508)
		die("lseek boot image");
	if (write(bfd, &ksize, 2) != 2)
		die("write boot image");
	/* write filesystem start block number into offset 506(2 bytes) of boot.bin */
	fsoff = ((8 + ksize) + 1) / 2;
	if (fsoff > 0xffff)
		die("file system offset is too large!");
	if (lseek(bfd, 506, SEEK_SET) != 506)
		die("lseek boot image");
	if (write(bfd, &fsoff, 2) != 2)
		die("write boot image");
	fwriteoffset(fsoff * 1024);
	/*
	 * write sectors of disk into offset 500(4 bytes) of boot.bin
	 *  (support max size of 2TB)
	 */
	if (lseek(bfd, 500, SEEK_SET) != 500)
		die("lseek boot image");
	if (write(bfd, &sectors, 4) != 4)
		die("write boot image");
}

static void close_image(void)
{
	close(bfd);
	close(kfd);
}

int main(int argc, char **argv)
{
	parse_args(argc, argv);
	open_image();
	build_image();
	close_image();
	return 0;
}

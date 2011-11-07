#include <ulib.h>

void parse_stat(char *file, struct file_stat *stat)
{
	printf("%s: ", file);
	switch (stat->mode & S_IFMT) {
	case S_IFSOCK:
		printf("socket");
		break;
	case S_IFLNK:
		printf("link");
		break;
	case S_IFREG:
		printf("regular");
		break;
	case S_IFBLK:
		printf("block");
		break;
	case S_IFDIR:
		printf("directory");
		break;
	case S_IFCHR:
		printf("char");
		break;
	case S_IFIFO:
		printf("fifo");
		break;
	default:
		printf("<unkown file type>");
	}
	printf("\nsize:%d bytes ino: %d refcnt:%d link:%d\n",
		stat->size, stat->inode, stat->iref, stat->link);
}

int main(int argc, char **argv)
{
	struct file_stat stat;
	int fd;
	int r = -1;
	if (argc != 2) {
		printf("Usage: stat file\n");
		return -1;
	}
	fd = open(argv[1], 0);
	if (fd < 0) {
		printf("stat: cannot open file: %s\n", argv[1]);
		return -1;
	}
	r = fstat(fd, &stat);
	if (r == 0)
		parse_stat(argv[1], &stat);
	else
		printf("stat: cannot get information of file:%s\n", argv[1]);
	close(fd);
	return r;
}

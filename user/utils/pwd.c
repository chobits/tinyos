#include <ulib.h>

#define BUF_SIZE 4096
static char buf[BUF_SIZE];

int main(int argc, char **argv)
{
	char *cwd;

	cwd = getcwd(buf, BUF_SIZE);
	if (!cwd)
		cwd = "error";

	printf("%s\n", cwd);

	return 0;
}

#include <ulib.h>

int main(int argc, char **argv)
{
	printf("[%d] %s", getpid(), argv[1]);
	exit(getpid());
	return 0;
}


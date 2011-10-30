#include <ulib.h>

int main(int argc, char **argv)
{
	int i;
	printf("------- user mode (stack %p)--------\n", &argc);
	for (i = 0; i < argc; i++)
		printf("argv[%d] %s\n", i, argv[i]);
	return 0xdeadbeef;
}


#include <syscall.h>

int fork(void)
{
	return usys_fork();
}

void yield(void)
{
	return usys_yield();
}

int getpid(void)
{
	return usys_getpid();
}

void exit(int status)
{
	usys_exit(status);
}

int wait(int *status)
{
	return usys_wait(status);
}

int execute(char *path, int argc, char **argv)
{
	return usys_execute(path, argc, argv);
}


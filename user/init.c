#include <ulib.h>

int main(int argc, char **argv)
{
	printf("-----------[In user mode]--------------\n");
#ifdef TEST_FORK
	int pid;
	while (1) {
		pid = fork();
		if (pid < 0) {
			printf("Fork error\n");
			break;
		} else if (pid == 0) {
			printf("I'm child %d\n", getpid());
			break;
		} else {
			printf("Create child %d\n", pid);
		}
	}
	while (1)
		yield();
#endif

#ifdef TEST_EXIT
	int pid;
	int rv;
refork:
	pid = fork();
	if (pid < 0) {
		printf("[%d] Fork error\n", getpid());
	} else if (pid == 0) {
refork_child:
		pid = fork();
		if (pid < 0) {
			printf("[%d] Fork error\n", getpid());
			printf("[%d] exit\n", getpid());
			exit(getpid());
		} else if (pid == 0) {
			printf("[%d] exit\n", getpid());
			exit(getpid());
		} else {
			printf("[%d] create %d\n", getpid(), pid);
		}
		goto refork_child;
	} else {
		printf("[%d] Create child %d and let it run!\n", getpid(), pid);
		yield();
		printf("[%d] waking up!\n", getpid());
		while ((pid = wait(&rv)) > 0)
			printf("[p] Child %d exit %d\n", pid, rv);
		goto refork;
	}
	while (1) ;
#endif

#ifdef TEST_EXEC
	char *argvs[] = { "hello", "world", "Haaaa!", NULL };
	int pid;
	int r;
refork:
	pid = fork();
	if (pid < 0) {
		printf("fork error\n");
	} else if (pid == 0) {
		r = execute("/hello", 3, argvs);
		printf("error for execute (%d)\n", r);
		while (1) ;
	}
	printf("[%d] Create task %d\n", getpid(), pid);
	yield();
	pid = wait(&r);
	if (pid > 0)
		printf("[%d] wait child %d return %x\n", getpid(), pid, r);
	else
		printf("[%d] wait error\n");
	goto refork;
#endif

#define TEST_EXEC_SAME
#ifdef TEST_EXEC_SAME
	char *argvs[] = { "hello", "world", "Haaaa!", NULL };
	int i;
	for (i = 0; i < argc; i++)
		printf("[%d] %s\n", i, argv[i]);
	execute("init", 3, argvs);
	printf("execute error\n");
	while (1) ;
#endif
	printf("I'm process %d\n", getpid());
	while (1) ;
	return 0;
}

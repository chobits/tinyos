#include <ulib.h>

void run_process_command(int argc, char **argv)
{
	int pid;
	int rpid, rv = 0;
	pid = fork();
	if (pid < 0) {
		printf("fork error\n");
		return;
	}
	/* child */
	if (pid == 0) {
		execute(argv[0], argc, argv);
		printf("execute %s error\n", argv[0]);
		exit(-1);
	}
	/* wait pid */
	rpid = wait(&rv);
	if (rpid != pid)
		printf("wait error pid: %d return %d\n", rpid, rv);
}


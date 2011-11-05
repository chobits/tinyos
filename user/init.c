#include <ulib.h>

#define iprintf(fmt, args...) printf("/init: " fmt, ##args)

int main(int argc, char **argv)
{
	iprintf("we are now in user mode\n");

#ifdef TEST_FORK
	int pid;
	while (1) {
		pid = fork();
		if (pid < 0) {
			iprintf("Fork error\n");
			break;
		} else if (pid == 0) {
			iprintf("I'm child %d\n", getpid());
			break;
		} else {
			iprintf("Create child %d\n", pid);
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
		iprintf("[%d] Fork error\n", getpid());
	} else if (pid == 0) {
refork_child:
		pid = fork();
		if (pid < 0) {
			iprintf("[%d] Fork error\n", getpid());
			iprintf("[%d] exit\n", getpid());
			exit(getpid());
		} else if (pid == 0) {
			char *as[] = { "/hello", "will exit\n", NULL };
			/*
			 * This bug is so strange:
			 *  Testing this script, you will get a panic!
			 * But add "iprintf("EXEC %d\n", pid);" here,
			 * this bug will disappear!
			 */
			execute("/hello", 2, as);
			while (1) ;
			exit(getpid());
		} else {
			iprintf("[%d] create %d\n", getpid(), pid);
		}
		goto refork_child;
	} else {
		iprintf("[%d] Create child %d and let it run!\n", getpid(), pid);
		yield();
		iprintf("[%d] waking up!\n", getpid());
		while ((pid = wait(&rv)) > 0)
			iprintf("[p] Child %d exit %d\n", pid, rv);
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
		iprintf("fork error\n");
	} else if (pid == 0) {
		r = execute("/hello", 3, argvs);
		iprintf("error for execute (%d)\n", r);
		while (1) ;
	}
	iprintf("[%d] Create task %d\n", getpid(), pid);
	yield();
	pid = wait(&r);
	if (pid > 0)
		iprintf("[%d] wait child %d return %x\n", getpid(), pid, r);
	else
		iprintf("[%d] wait error\n");
	goto refork;
#endif

#ifdef TEST_EXEC_SAME
	char *argvs[] = { "hello", "world", "Haaaa!", NULL };
	int i;
	for (i = 0; i < argc; i++)
		iprintf("[%d] %s\n", i, argv[i]);
	execute("init", 3, argvs);
	iprintf("execute error\n");
	while (1) ;
#endif

#ifdef TEST_OPEN_CLOSE
	int fd;
	char buf[4];
	int size, i;
	fd = open("/text", 0);
	while ((size = read(fd, buf, 4)) > 0)
		iprintf("%*s", size, buf);
	if (size < 0) {
		iprintf("read return %d\n", size);
		goto err;
	}
	iprintf("read ok!\n");
	if (lseek(fd, 0, SEEK_SET) != 0) {
		iprintf("lseek error\n");
		goto err;
	}
	/* (7 + 512 + 1)*1024 */
	i = 832480;
	while (i > 0) {
		if ((size = write(fd, "123456789\n", 10)) != 10) {
			iprintf("write error %d\n", size);
			goto err;
		}
		i -= 10;
	}
	iprintf("write ok!\n");
	fsync(fd);
	iprintf("fsync ok!\n");
	fd = close(fd);
err:
	while (1) ;
#endif

#ifdef TEST_GETS
	char buf[64];
	int size;
	while ((size = gets(buf, 64)) > 0)
		iprintf("[%d]%*s", size, size, buf);
	iprintf("ERROR gets return %d\n", size);
	while (1) ;
#endif

#define TEST_SHELL
#ifdef TEST_SHELL
	int pid, rv;
	pid = fork();
	if (pid < 0) {
		iprintf("Fork error\n");
	} else if (pid == 0) {
		char *as[] = { "/sh", NULL };
		execute("/sh", 1, as);
		iprintf("execve error\n");
	} else {
		pid = wait(&rv);
		if (pid > 0)
			iprintf("/sh(%d) exit %d\n", pid, rv);
		else
			iprintf("/sh exit error\n");
	}

	while (1)
		yield();
#endif
	iprintf("I'm process %d\n", getpid());
	while (1) ;
	return 0;
}

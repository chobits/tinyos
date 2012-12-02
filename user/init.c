#include <ulib.h>

#define iprintf(fmt, args...) printf("/init: " fmt, ##args)

void test_pit_sched(void)
{
	if (fork() == 0) {
		if (fork() == 0) {
			while (1) {
				printf("C");
				yield();	/* test interrupt_level in schedule() */
			}
		} else {
			while (1) {
				printf("X");
			}
		}
	} else {
		while (1)
			printf("P");		/* test schedule() in timer interrupt */

	}
}

void test_fork(void)
{
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
	while (1); yield();
		//iprintf("> pid %d <\n", getpid());

}

void test_exit(void)
{
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

}

void test_exec(void)
{
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
}

void test_open_close(void)
{
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

}

void test_gets(void)
{
	char buf[64];
	int size;
	while ((size = gets(buf, 64)) > 0)
		iprintf("[%d]%*s", size, size, buf);
	iprintf("ERROR gets return %d\n", size);
	while (1) ;
}


int main(int argc, char **argv)
{
	iprintf("in user mode\n");

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

	while (1) ;
	return 0;
}

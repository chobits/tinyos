/* shell builtin command */
#include <ulib.h>
#include <string.h>

struct builtin_command {
	char *cmd;
	char *info;
	void (*func)(int, char **);
};

#define BUILTIN_CMDS	(sizeof(cmds) / sizeof(cmds[0]))

static void builtin_cd(int argc, char **argv);
static void builtin_echo(int argc, char **argv);
static void builtin_pid(int argc, char **argv);
static void builtin_help(int argc, char **argv);
static void builtin_exit(int argc, char **argv);

static struct builtin_command cmds[] = {
	{ "help", "display manual of this shell", builtin_help },
	{ "cd", "change current directory", builtin_cd },
	{ "echo", "display a line of text", builtin_echo },
	{ "pid", "display the ID of shell", builtin_pid },
	{ "exit", "cause the shell to exit", builtin_exit },
};

static void builtin_help(int argc, char **argv)
{
	int i;
	for (i = 0; i < BUILTIN_CMDS; i++)
		printf("%-8s - %s\n", cmds[i].cmd, cmds[i].info);
}

static void builtin_cd(int argc, char **argv)
{
	char *dir = "No dir";
	if (argc > 2) {
		printf("too many arguments for cd\n");
		return;
	}
	if (argc == 2) {
		dir = argv[1];
	} else {
		dir = "/";
	}
	chdir(dir);
}

static void builtin_echo(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++)
		printf("%s ", argv[i]);
	printf("\n");
}

static void builtin_pid(int argc, char **argv)
{
	printf("%d\n", getpid());
}

static void builtin_exit(int argc, char **argv)
{
	printf("exit\n");
	exit(0);
}

int run_builtin_command(int argc, char **argv)
{
	int i;
	for (i = 0; i < BUILTIN_CMDS; i++)
		if (strcmp(argv[0], cmds[i].cmd) == 0)
			goto found;
	return -1;
found:
	cmds[i].func(argc, argv);
	return 0;
}


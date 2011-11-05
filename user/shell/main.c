/*
 * simple shell for net stack debug and monitor
 */
#include <ulib.h>

int run_builtin_command(int, char **);
int run_process_command(int, char **);

int get_line(char *, int);
int parse_line(char *, int, char **);

static char *prompt = "[shell]";

static void run_command(int argc, char **argv)
{
	if (run_builtin_command(argc, argv) < 0)
		run_process_command(argc, argv);
}

static _inline void print_prompt(void)
{
	printf("%s: ", prompt);
}

void shell_run(char *prompt_str)
{
	char linebuf[128];
	int linelen;
	char *argv[16];
	int argc;

	/* shell master */
	if (prompt_str && *prompt_str)
		prompt = prompt_str;
	while (1) {
		print_prompt();
		linelen = get_line(linebuf, 128);
		argc = parse_line(linebuf, linelen, argv);
		if (argc > 0)
			run_command(argc, argv);
		else if (argc < 0)
			printf("-shell: too many arguments\n");
	}
}

int main(int argc, char **argv)
{
	shell_run(NULL);
	return 0;
}


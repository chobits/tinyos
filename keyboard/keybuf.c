#include <wait_queue.h>
#include <keyboard.h>
#include <string.h>
#include <print.h>
#include <task.h>

WAIT_QUEUE(key_wait_queue);
static int key_free;	/* free index */
static int eof;
static char key_buf[KEY_BUF_SIZE];

void key_putc(char c)
{
	int out = 1;
	if (!key_wait_queue.num)
		return;
	/* EOF */
	if (c == '\0') {
		out = 0;
		eof = 1;
		key_buf[key_free] = c;
	} else if (c == '\b') {
		if (key_free > 0)
			key_free--;
		else
			out = 0;
	} else {
		key_buf[key_free] = c;
		key_free++;
	}
	/* output to video */
	if (out)
		text_putc(c);
	/* wake up process who is waiting for the line buffer */
	if (c == '\0' || c == '\n' || key_free >= KEY_BUF_SIZE)
		wake_up(&key_wait_queue);
}

int sys_gets(char *buf, int size)
{
	if (size < 0)
		return size;
	while (key_free <= 0 && !eof)
		sleep_on(&key_wait_queue);

	size = min(size, key_free);
	key_free = 0;
	eof = 0;
	memcpy(buf, key_buf, size);
	return size;
}


#include <wait_queue.h>
#include <keyboard.h>
#include <string.h>
#include <task.h>

WAIT_QUEUE(key_wait_queue);
static int key_free;	/* free index */
static char key_buf[KEY_BUF_SIZE];

void key_putc(char c)
{
	if (!key_wait_queue.num)
		return;

	if (c == '\b') {
		if (key_free > 0)
			key_free--;
	} else {
		key_buf[key_free] = c;
		key_free++;
	}

	if (c == '\n' || key_free >= KEY_BUF_SIZE)
		wake_up(&key_wait_queue);
}

int sys_gets(char *buf, int size)
{
	if (size < 0)
		return size;
	while (key_free <= 0)
		sleep_on(&key_wait_queue);
	size = min(size, key_free);
	key_free = 0;
	memcpy(buf, key_buf, size);
	return size;
}


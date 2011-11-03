#include <keyboard.h>
#include <print.h>
#include <8259A.h>
#include <int.h>
#include <x86.h>

extern unsigned char scancode_types[];
extern char scancode_ascii[];
extern char scancode_shift_ascii[];
void key_putc(char);

static struct key_desc keys;

void keyboard_interrupt(struct regs *reg)
{
	/*
	 * We must read this scan code from the buffer
	 * otherwise the keyboard interrupt will be closed.
	 */
	u8 scan_code = inb(0x60);
	u8 press = 1;
	int type;
	char c;
	/* Just skip prefix code: 0xE0 */
	if (scan_code == KEY_E0)
		return;
	/* scan code of pause key: 0xe1 0x1d 0x45 0xe1 0x4d 0xc5 */
	if (scan_code == KEY_E1 || keys.pause) {
		keys.pause = 1;
		if (scan_code == 0xc5)
			keys.pause = 0;
		return;
	}

	if (scan_code & KEY_RELEASE_BIT) {
		scan_code &= 0x7f;
		press = 0;
	}

	/* get scancode types */
	type = scancode_types[scan_code];
	/* handle none code */
	if (press && type == KEY_NONE) {
		printk("None scan code: %x\n", scan_code);
		return;
	}
	/* handle prefix code */
	if (scancode_types[scan_code] == KEY_PREFIX) {
		switch (scan_code) {
		case KEY_ESC:
			keys.esc = press;
			break;
		case KEY_CAPSLOCK:
			if (press)
				keys.capslock = !keys.capslock;
			break;
		case KEY_LSHIFT:
		case KEY_RSHIFT:
			keys.shift = press;
			break;
		case KEY_START:
			keys.start = press;
			break;
		case KEY_ALT:
			keys.alt = press;
			break;
		case KEY_CTRL:
			keys.ctrl = press;
			break;
		}
		return;
	}
	/* handle acsii scan code */
	if (press && type == KEY_ASCII) {
		if (keys.capslock || keys.shift)
			c = scancode_shift_ascii[scan_code];
		else
			c = scancode_ascii[scan_code];
		/* EOF */
		if ((c == 'd' || c == 'D') && keys.ctrl)
			c = '\0';
		key_putc(c);
	}
}

void keyboard_init(void)
{
	/* open keyboard interrupt */
	unmask_8259A(IRQ_KEYBOARD);
}

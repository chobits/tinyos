#include <compile.h>
#include <string.h>
#include <print.h>
#include <types.h>
#include <x86.h>
#include <mm.h>

/* Text Parameters */
#define TEXT_BASE	0xb8000
#define TEXT_VBASE	((void *)VADDR(TEXT_BASE))
#define TEXT_END	(TEXT_BASE + TEXT_SCREENS * TEXT_SCR_SIZE)
#define TEXT_VEND	((void *)VADDR(TEXT_END))
#define TEXT_SIZE	(0xc0000 - 0xb8000)
#define TEXT_LINES	25
#define TEXT_ROWS	80
#define TEXT_CHR_SIZE	2
#define TEXT_LINE_SIZE	(TEXT_ROWS * TEXT_CHR_SIZE)
#define TEXT_SCR_CHRS	(TEXT_ROWS * TEXT_LINES)
#define TEXT_SCR_SIZE	(TEXT_SCR_CHRS * TEXT_CHR_SIZE)
/* reserve one blank line for scrolling down last screen */
#define TEXT_SCREENS	((TEXT_SIZE - TEXT_LINE_SIZE) / TEXT_SCR_SIZE)
/* I/O Port */
#define TEXT_REG_INDEX	0x3d4
#define TEXT_REG_VALUE	0x3d5
/* Function index */
#define INDEX_SCREEN_HIGH	0x0c
#define INDEX_SCREEN_LOW	0x0d
#define INDEX_CURSOR_HIGH	0x0e
#define INDEX_CURSOR_LOW	0x0f
/* Special Chars */
#define TEXT_TAB_SIZE	(8 * TEXT_CHR_SIZE)
/* Text Char Mode */
#define TEXT_BLACK_CHRMODE		0x00
#define TEXT_BLUE_CHRMODE		0x01
#define TEXT_GREEN_CHRMODE		0x02
#define TEXT_CYAN_CHRMODE		0x03
#define TEXT_RED_CHRMODE		0x04
#define TEXT_MAGENTA_CHRMODE		0x05
#define TEXT_BROWN_CHRMODE		0x06
#define TEXT_LIGHTGRAY_CHRMODE		0x07
#define TEXT_DARKGRAY_CHRMODE		0x08
#define TEXT_LIGHTBLUE_CHRMODE		0x09
#define TEXT_LIGHTGREEN_CHRMODE		0x0a
#define TEXT_LIGHTCYAN_CHRMODE		0x0b
#define TEXT_LIGHTRED_CHRMODE		0x0c
#define TEXT_LIGHTMAGENTA_CHRMODE	0x0d
#define TEXT_YELLOW_CHRMODE		0x0e
#define TEXT_WHITE_CHRMODE		0x0f
#define TEXT_DEF_CHRMODE		TEXT_WHITE_CHRMODE

/*
 * Text Screen Buffer Memory:
 *
 *   .-----total screen buffer------.
 *   |                              |
 *   |      .-current screen-.      |
 *   |      |                |      |
 *   |      .-offset-.       |      |
 *   |      |        |       |      |
 *   +------+--------+-------+------+
 *   |      |        |
 * base  screen    cursor
 *
 */
static void *text_base;		/* base address of total text memory */
static void *text_screen;	/* base address of screen memory */
static void *text_cursor;

static _inline void set_cursor(u32 offset)
{
	outb_p(TEXT_REG_INDEX, INDEX_CURSOR_LOW);
	outb_p(TEXT_REG_VALUE, (u16)(offset & 0xff));
	outb_p(TEXT_REG_INDEX, INDEX_CURSOR_HIGH);
	outb_p(TEXT_REG_VALUE, (u16)((offset >> 8) & 0xff));
}

static void update_cursor(void)
{
	set_cursor((text_cursor - text_base) / TEXT_CHR_SIZE);
}

static _inline void text_set_screen(u32 offset)
{
	outb_p(TEXT_REG_INDEX, INDEX_SCREEN_LOW);
	outb_p(TEXT_REG_VALUE, (u16)(offset & 0xff));
	outb_p(TEXT_REG_INDEX, INDEX_SCREEN_HIGH);
	outb_p(TEXT_REG_VALUE, (u16)((offset >> 8) & 0xff));
}

static void update_screen(void)
{
	text_set_screen((text_screen - text_base) / TEXT_CHR_SIZE);
}

static _inline void clear_vmem(void *src, int chars)
{
	u16 mode_char;
	mode_char = (TEXT_DEF_CHRMODE << 8) | ' ';
	wmemset(src, mode_char, chars);
}

/* clear current screen without altering cursor */
static void clear_screen(void)
{
	clear_vmem(text_screen, TEXT_SCR_CHRS);
}

static void text_scroll_down(void)
{
	if (text_cursor >= TEXT_VEND) {
		/*
		 * We have reserved one blank line after TEXT_END,
		 * the blank line is copied to last line of new screen.
		 * So we dont need to clear the last line manually.
		 */
		text_screen += TEXT_LINE_SIZE;
		memcpy(text_base, text_screen, TEXT_SCR_SIZE);
		text_cursor -= (text_screen - text_base);
		text_screen = text_base;
		/* But we should clear other screens. */
		clear_vmem(text_base + TEXT_SCR_SIZE, TEXT_SCR_CHRS * (TEXT_SCREENS - 1));
	} else {
		text_screen += TEXT_LINE_SIZE;
	}
	update_screen();
	update_cursor();
}

static _inline void text_put_char(char c)
{
	((char *)text_cursor)[0] = c;
	((char *)text_cursor)[1] = TEXT_DEF_CHRMODE;
}

static void text_handle_char(char c)
{
	switch (c) {
	case '\b':
		if (text_cursor > text_screen) {
			text_cursor -= TEXT_CHR_SIZE;
			text_put_char(' ');
		}
		break;
	case '\t':
		text_cursor = ALIGN_UP(text_cursor + TEXT_CHR_SIZE,
					TEXT_TAB_SIZE);
		break;
	case '\n':
		text_cursor += TEXT_LINE_SIZE;
		text_cursor -= (text_cursor - TEXT_VBASE) % TEXT_LINE_SIZE;
		break;
	case '\r':
		text_cursor += TEXT_LINE_SIZE;
		break;
	default:
		text_put_char(c);
		text_cursor += TEXT_CHR_SIZE;
		break;
	}
	if (text_cursor >= text_screen + TEXT_SCR_SIZE)
		text_scroll_down();
}

void text_putc(char c)
{
	text_handle_char(c);
	update_cursor();
}

int text_nputs(char *s, int n)
{
	int i = 0;
	while (*s && i < n) {
		text_handle_char(*s);
		s++;
		i++;
	}
	update_cursor();
	return i;
}

int text_puts(char *s)
{
	int i = 0;
	while (*s) {
		text_handle_char(*s++);
		i++;
	}
	update_cursor();
	return i;
}

void text_init(void)
{
	/*
	 * This is a simple initialization:
	 *  Some configure is default according to BIOS initialization.
	 *  For robustness, we should detect and set VGA mode manually.
	 *  But it can work well now.
	 */
	text_base = TEXT_VBASE;
	text_screen = text_base;
	text_cursor = text_base;
	update_cursor();
	clear_screen();	
	printk("Video: init text mode\n");
}

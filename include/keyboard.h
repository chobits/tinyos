#ifndef __KEYBOARD_H
#define __KEYBOARD_H

struct key_desc {
	unsigned int	esc:1,
			capslock:1,
			shift:1,
			ctrl:1,
			alt:1,
			start:1,
			pause:1;
};

#define KEY_BUF_SIZE	512

/* This key map is from bochs us-keymap */
#define KEY_NONE	0x00
#define KEY_PREFIX	0x01
#define KEY_ASCII	0x02
#define KEY_SPECIAL	0x04

#define KEY_RELEASE_BIT	0x80
#define KEY_E0		0xe0

/* pause code: e1 1d 45 e1 4d c5 */
#define KEY_E1		0xe1

#define KEY_ESC		0x1
#define KEY_1		0x2
#define KEY_2		0x3
#define KEY_3		0x4
#define KEY_4		0x5
#define KEY_5		0x6
#define KEY_6		0x7
#define KEY_7		0x8
#define KEY_8		0x9
#define KEY_9		0xa
#define KEY_0		0xb
#define KEY_MINUS	0xc
#define KEY_EUQAL	0xd
#define KEY_BKSP	0xe
#define KEY_TAB		0xf
#define KEY_Q		0x10
#define KEY_W		0x11
#define KEY_E		0x12
#define KEY_R		0x13
#define KEY_T		0x14
#define KEY_Y		0x15
#define KEY_U		0x16
#define KEY_I		0x17
#define KEY_O		0x18
#define KEY_P		0x19
//#define KEY_[		0x1a
//#define KEY_]		0x1b
#define KEY_ENTER	0x1c
#define KEY_CTRL	0x1d
#define KEY_A		0x1e
#define KEY_S		0x1f
#define KEY_D		0x20
#define KEY_F		0x21
#define KEY_G		0x22
#define KEY_H		0x23
#define KEY_J		0x24
#define KEY_K		0x25
#define KEY_L		0x26
#define KEY_SEMI	0x27
//#define KEY_'		0x28
//#define KEY_`		0x29
#define KEY_LSHIFT	0x2A
#define KEY_BACKSLASH	0x2B	/* \ */
#define KEY_Z		0x2C
#define KEY_X		0x2D
#define KEY_C		0x2E
#define KEY_V		0x2F
#define KEY_B		0x30
#define KEY_N		0x31
#define KEY_M		0x32
//#define KEY_<		0x33
//#define KEY_>		0x34
#define KEY_SLASH	0x35	/* / ? */
#define KEY_RSHIFT	0x36
#define KEY_37
#define KEY_ALT		0x38
#define KEY_SPACE	0x39
#define KEY_CAPSLOCK	0x3a
#define KEY_F1		0x3b
#define KEY_F2		0x3c
#define KEY_F3		0x3d
#define KEY_F4		0x3e
#define KEY_F5		0x3f
#define KEY_F6		0x40
#define KEY_F7		0x41
#define KEY_F8		0x42
#define KEY_F9		0x43
#define KEY_F10		0x44
#define KEY_PAUSE	0x45
#define KEY_SCRLK	0x46
#define KEY_HOME	0x47
#define KEY_UP		0x48
#define KEY_PGUP	0x49
#define KEY_4A
#define KEY_LEFT	0x4b
#define KEY_4C
#define KEY_RIGHT	0x4d
#define KEY_4E
#define KEY_END		0x4f
#define KEY_DOWN	0x50
#define KEY_PGDN	0x51
#define KEY_INSERT	0x52
#define KEY_DELETE	0x53
#define KEY_54
#define KEY_55
#define KEY_56
#define KEY_F11		0x57
#define KEY_F12		0x58
#define KEY_59
#define KEY_5A
#define KEY_START	0x5b
#define KEY_5C
#define KEY_BOOK	0x5d

#endif	/* keyboard.h */

#include <keyboard.h>

#define P KEY_PREFIX
#define N KEY_NONE
#define A KEY_ASCII
#define S KEY_SPECIAL

unsigned char scancode_types[128] = {
/* 00:         ESC      1       2       3       4       5       6     */
	N,	P,	A,	A,	A,	A,	A,	A,
/* 08:  7       8       9       0       -       =   backspace  tab    */
	A,	A,	A,	A,	A,	A,	A,	A,
/* 10:  q       w       e       r       t       y       u       i     */
	A,	A,	A,	A,	A,	A,	A,	A,
/* 18:  o       p       [       ]     enter ctrl/pause  a       s     */
	A,	A,	A,	A,	A,	P,	A,	A,
/* 20:  d       f       g       h       j       k       l       ;     */
	A,	A,	A,	A,	A,	A,	A,	A,
/* 28:  '       `     lshirt    \       z       x       c       v     */
	A,	A,	P,	A,	A,	A,	A,	A,
/* 30:  b       n       m      , <     . >     / ?     rshift         */
	A,	A,	A,	A,	A,	A,	P,	N,
/* 38: alt    space capslock    F1      F2      F3      F4      F5    */
	P,	A,	P,	S,	S,	S,	S,	S,
/* 40:  F6      F7      F8      F9     F10    pause   scrlk    home   */
	S,	S,	S,	S,	S,	S,	S,	S,
/* 48:  up     pgup            left           right            end    */
	S,	S,	N,	S,	N,	S,	N,	S,
/* 50: down    pgdn   insert  delete                           F11    */
	S,	S,	S,	S,	N,	N,	N,	S,
/* 58: F12                     start        book,mouse                */
	S,	N,	N,	S,	N,	S,	N,	N,
/* 60: */
	N,	N,	N,	N,	N,	N,	N,	N,
/* 68: */
	N,	N,	N,	N,	N,	N,	N,	N,
/* 70: */
	N,	N,	N,	N,	N,	N,	N,	N,
/* 78: */
	N,	N,	N,	N,	N,	N,	N,	N,
};

#define UNA	0	/* unascii */
char scancode_ascii[0x40] = {
	UNA,	UNA,	'1',	'2',	'3',	'4',	'5',	'6',
	'7',	'8',	'9',	'0',	'-',	'=',	'\b',	'\t',
	'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
	'o',	'p',	'[',	']',	'\n',	UNA,	'a',	's',
	'd',	'f',	'g',	'h',	'j',	'k',	'l',	';',
	'\'',	'`',	UNA,	'\\',	'z',	'x',	'c',	'v',
	'b',	'n',	'm',	',',	'.',	'/',	UNA,	UNA,
	UNA,	' ',	UNA,	UNA,	UNA,	UNA,	UNA,	UNA,
};

char scancode_shift_ascii[0x40] = {
	UNA,	UNA,	'!',	'@',	'#',	'$',	'%',	'^',
	'&',	'*',	'(',	')',	'_',	'+',	'\b',	'\t',
	'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
	'O',	'P',	'{',	'}',	'\n',	UNA,	'A',	'S',
	'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
	'"',	'~',	UNA,	'|',	'Z',	'X',	'C',	'V',
	'B',	'N',	'M',	'<',	'>',	'?',	UNA,	UNA,
	UNA,	' ',	UNA,	UNA,	UNA,	UNA,	UNA,	UNA,
};


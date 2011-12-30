/*
 * Programmbale Interval Timer(PIT) chip (8253/8254 chip)
 */
#ifndef __PIT_H
#define __PIT_H

#define PIT_CH0		0x40	/* Channle 0 data port(read/write) */
#define PIT_CH1		0x41	/* Channle 1 data port(read/write) */
#define PIT_CH2		0x42	/* Channle 2 data port(read/write) */
#define PIT_MODE	0x43	/* Mode/Command register(write only, a read is ignored) */

/*
 * The Mode/Command register at I/O address 0x43 contains the following:
 *    Bits      Usage
 *    6, 7      Select channel :
 *                   (7 6)
 *                    0 0 = Channel 0
 *                    0 1 = Channel 1
 *                    1 0 = Channel 2
 *                    1 1 = Read-back command (8254 only)
 *    4, 5      Access mode :
 *                   (5 4)
 *                    0 0 = Latch count value command
 *                    0 1 = Access mode: lobyte only
 *                    1 0 = Access mode: hibyte only
 *                    1 1 = Access mode: lobyte/hibyte
 *    1, 2, 3   Operating mode :
 *                   (3 2 1)
 *                    0 0 0 = Mode 0 (interrupt on terminal count)
 *                    0 0 1 = Mode 1 (hardware re-triggerable one-shot)
 *                    0 1 0 = Mode 2 (rate generator)
 *                    0 1 1 = Mode 3 (square wave generator)
 *                    1 0 0 = Mode 4 (software triggered strobe)
 *                    1 0 1 = Mode 5 (hardware triggered strobe)
 *                    1 1 0 = Mode 2 (rate generator, same as 010b)
 *                    1 1 1 = Mode 3 (square wave generator, same as 011b)
 *    0         BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
 */
#define PIT_MODE_CH0	0x00
#define PIT_MODE_CH1	0x40
#define PIT_MODE_CH2	0x80
#define PIT_MODE_RB	0xc0

#define PIT_MODE_LATCH	0x00
#define PIT_MODE_LOW	0x10
#define PIT_MODE_HIGH	0x20
#define PIT_MODE_LOHI	0x30

#define PIT_MODE_0	0x00
#define PIT_MODE_1	0x02
#define PIT_MODE_2	0x04
#define PIT_MODE_3	0x06
#define PIT_MODE_4	0x08
#define PIT_MODE_5	0x09
#define PIT_MODE_6	0x0a
#define PIT_MODE_7	0x0e

#define TIMER_FREQ	1000	/* timer interrupt frequence: 100 times/sec */
#define PIT_FREQ	1193182	/* 8253 chip frequence: 1.193181666MHz */
#define FREQ_DIV	(PIT_FREQ / TIMER_FREQ)

#endif	/* pit.h */

#include "main.h"

static volatile unsigned char vblank;
static void interrupt(void) __naked {
    __asm__("di");
    __asm__("push af");
    __asm__("ld a, #1");
    __asm__("ld (_vblank), a");
    __asm__("pop af");
    __asm__("ei");
    __asm__("reti");
}

static void setup_irq(void) {
    __asm__("di");
    unsigned short addr;
    for (addr = 0xfe00; addr <= 0xff00; addr++) {
	MEM(addr) = 0xfd;
    }
    MEM(0xfdfd) = 0xc3;
    MEM(0xfdfe) = ADDR(&interrupt) & 0xff;
    MEM(0xfdff) = ADDR(&interrupt) >> 8;
    __asm__("push af");
    __asm__("ld a, #0xfe");
    __asm__("ld i, a");
    __asm__("pop af");
    __asm__("im 2");
    __asm__("ei");
}

static void out_fe(unsigned char a) {
    __asm__("out (#0xfe), a");
}

void main(void) {
    __asm__("ld sp, #0xFDFC");

    setup_irq();

    unsigned short addr;
    for (addr = 0x4000; addr < 0x5800; addr++) {
	MEM(addr) = addr & 0x100 ? 0xaa : 0x55;
    }
    for (addr = 0x5800; addr < 0x5b00; addr++) {
	MEM(addr) = 0x47;
    }

    signed char d = 1;
    unsigned short i, r = 200;
    while (1) {
	out_fe(0x10);
	for (i = 0; i < r; i++) { }
	out_fe(0x00);
	for (i = 0; i < r; i++) { }

	r = r + d;
	if (r == 100) d = +1;
	if (r == 300) d = -1;
    }
}

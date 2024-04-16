#include "main.h"

static volatile byte vblank;
static void interrupt(void) __naked {
    __asm__("di");
    __asm__("push af");
    __asm__("ld a, #1");
    __asm__("ld (_vblank), a");
    __asm__("pop af");
    __asm__("ei");
    __asm__("reti");
}

static void memset(word addr, byte data, word len) {
    while (len-- > 0) { BYTE(addr++) = data; }
}

static void setup_irq(void) {
    __asm__("di");
    BYTE(0xfdfd) = 0xc3;
    WORD(0xfdfe) = ADDR(&interrupt);
    memset(0xfe00, 0xfd, 0x101);
    __asm__("push af");
    __asm__("ld a, #0xfe");
    __asm__("ld i, a");
    __asm__("pop af");
    __asm__("im 2");
    __asm__("ei");
}

static void out_fe(byte a) {
    __asm__("out (#0xfe), a"); a;
}

void main(void) {
    __asm__("ld sp, #0xFDFC");

    setup_irq();

    memset(0x4000, 0xc0, 0x1800);
    memset(0x5800, 0x47, 0x0300);

    int8 d = 1;
    word i, r = 200;
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

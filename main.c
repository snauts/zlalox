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

static byte in_fe(byte a) __naked {
    __asm__("in a, (#0xfe)"); a;
    __asm__("ret");
}

static byte press_z(void) {
    return !(in_fe(0xfe) & 0x02);
}

static byte press_x(void) {
    return !(in_fe(0xfe) & 0x04);
}

static void slow_pixel(byte x, byte y) {
    y = ((y & 7) << 3) | ((y >> 3) & 7) | (y & 0xC0);
    word addr = (y << 5) + (x >> 3);
    byte pixel = 0x80 >> (x & 7);
    BYTE(0x4000 + addr) ^= pixel;
}

static void clear_screen(void) {
    memset(0x4000, 0x00, 0x1B00);
}

static void track_color(void) {
    memset(0x5800, 0x01, 0x300);
    memset(0x5880, 0x05, 0x260);
    memset(0x5900, 0x07, 0x1C0);
    memset(0x5980, 0x47, 0x120);
}

static void track_border(void) {
    for (word addr = 0x4000; addr < 0x5800; addr += 0x20) {
	BYTE(addr + 0x07) = 0x03;
	BYTE(addr + 0x18) = 0xC0;
    }
}

static byte pos;
static int8 dir;
static void control(void) {
    dir = 0;
    if (press_z()) {
	dir = -1;
    }
    if (press_x()) {
	dir = +1;
    }
    pos += dir;
}

static const word pixels[] = {
    0x00A0, 0x0050, 0x0028, 0x0014,
    0x000A, 0x0005, 0x8002, 0x4001
};

static void draw_straight(void) {
    word addr = 0x5180 + (pos >> 3);
    word data = pixels[pos & 0x07];
    for (byte n = 0; n < 5; n++) {
	WORD(addr) ^= data;
	addr += 0x100;
    }
}

static const word pixels_L[] = {
    0x00A0, 0x00A0, 0x0050, 0x0028,
    0x0050, 0x0050, 0x0028, 0x0014,
    0x0028, 0x0028, 0x0014, 0x000A,
    0x0014, 0x0014, 0x000A, 0x0005,
    0x000A, 0x000A, 0x0005, 0x8002,
    0x0005, 0x0005, 0x8002, 0x4001,
    0x8002, 0x8002, 0x4001, 0xA000,
    0x4001, 0x4001, 0xA000, 0x5000
};

static const word pixels_R[] = {
    0x0028, 0x0028, 0x0050, 0x00A0,
    0x0014, 0x0014, 0x0028, 0x0050,
    0x000A, 0x000A, 0x0014, 0x0028,
    0x0005, 0x0005, 0x000A, 0x0014,
    0x8002, 0x8002, 0x0005, 0x000A,
    0x4001, 0x4001, 0x8002, 0x0005,
    0xA000, 0xA000, 0x4001, 0x8002,
    0x5000, 0x5000, 0xA000, 0x4001
};

static void draw_side(byte x, word data) {
    data += (x & 0x07) << 3;
    word addr = 0x5280 + (x >> 3);
    for (byte n = 0; n < 4; n++) {
	WORD(addr) ^= WORD(data);
	addr += 0x100;
	data += 0x02;
    }
}

static void draw_player(void)  {
    switch (dir) {
    case -1:
	draw_side(pos, ADDR(pixels_L));
	break;
    case +1:
	draw_side(pos - 2, ADDR(pixels_R));
	break;
    default:
	draw_straight();
	break;
    }
}

static void init_variables(void) {
    pos = 127;
    dir = 0;
}

void game_loop(void) {
    for (;;) {
	control();
	draw_player();
	vblank = 0;
	while (!vblank) { }
	draw_player();
    }
}

void main(void) {
    __asm__("ld sp, #0xFDFC");

    setup_irq();
    clear_screen();
    init_variables();
    track_border();
    track_color();
    game_loop();
}

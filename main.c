#include "main.h"
#include "data.h"

#define BORDER 10

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

static word map_y[192];

static void slow_pixel(byte x, byte y) {
    BYTE(map_y[y] + (x >> 3)) ^= 0x80 >> (x & 7);
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
	BYTE(addr + 31 - BORDER) = 0xC0;
	BYTE(addr + BORDER) = 0x03;
    }
}

static void error_at(const char *msg, byte y) {
    y = y << 3;
    byte done = 0;
    for (byte x = 0; x < BORDER; x++) {
	word addr = 0x3C00;
	char symbol = msg[x];
	if (symbol == 0) done = 1;
	addr += (done ? 0x100 : (symbol << 3));
	for (byte i = 0; i < 8; i++) {
	    BYTE(map_y[y + i] + x) = BYTE(addr + i);
	}
	BYTE(0x5800 + (y << 2) + x) = 0x07;
    }
}

static byte err;
static void error_str(const char *msg) {
    error_at(msg, err);
    if (++err >= 24) err = 0;
    error_at("----------------", err);
}

static char to_hex(byte digit) {
    return (digit < 10) ? '0' + digit : 'A' + digit - 10;
}

static void error_num(word num) {
    char msg[] = "0x0000";
    for (byte i = 5; i >= 2; i--) {
	msg[i] = to_hex(num & 0xf);
	num = num >> 4;
    }
    error_str(msg);
}

static byte pos;
static int8 dir;
static void control(void) {
    byte port = in_fe(0xfe);
    dir = 0;
    if ((port & 2) == 0) dir--;
    if ((port & 4) == 0) dir++;
    pos += dir;
}

static byte detect, collision;
static void draw_straight(void) {
    word addr = 0x5180 + (pos >> 3);
    word data = pixels[pos & 0x07];
    for (byte n = 0; n < 5; n++) {
	word screen = WORD(addr);
	if (detect && (screen & data)) collision = 1;
	WORD(addr) = screen ^ data;
	addr += 0x100;
    }
}

static void draw_side(byte x, word data) {
    data += (x & 0x07) << 3;
    word addr = 0x5280 + (x >> 3);
    for (byte n = 0; n < 4; n++) {
	word screen = WORD(addr);
	word sprite = WORD(data);
	if (detect && (screen & sprite)) collision = 1;
	WORD(addr) = screen ^ sprite;
	addr += 0x100;
	data += 0x02;
    }
}

static void draw_player(byte check)  {
    detect = check;
    collision = 0;
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
    if (collision) {
	error_str("HIT-POS");
	error_num(pos);
    }
}

static void init_variables(void) {
    pos = 127;
    dir = 0;
    err = 0;
}

static void wait_vblank(void) {
    vblank = 0;
    while (!vblank) { }
}

static void scroll_snow(void) {
}

static void next_pattern(void) {
}

static void game_loop(void) {
    wait_vblank();
    next_pattern();
    for (;;) {
	control();
	scroll_snow();
	draw_player(1);
	wait_vblank();
	draw_player(0);
    }
}

static void prepare(void) {
    for (byte y = 0; y < 192; y++) {
	byte f = ((y & 7) << 3) | ((y >> 3) & 7) | (y & 0xC0);
	map_y[y] = 0x4000 + (f << 5);
    }
}

void main(void) {
    __asm__("ld sp, #0xFDFC");

    setup_irq();
    prepare();

    for (;;) {
	clear_screen();
	init_variables();
	track_border();
	track_color();

	game_loop();
    }
}

#include "main.h"
#include "data.h"

#include "level.h"

#define WIDTH	10
#define BORDER	10

void main(void);

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

static void __sdcc_call_hl(void) __naked {
    __asm__("jp (hl)");
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
    BYTE(map_y[y] + (x >> 3)) ^= shift_R[x & 7];
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

static void put_char(char symbol, byte x, byte y, byte color) {
    y = y << 3;
    word addr = 0x3C00 + (symbol << 3);
    for (byte i = 0; i < 8; i++) {
	BYTE(map_y[y + i] + x) = BYTE(addr + i);
    }
    BYTE(0x5800 + (y << 2) + x) = color;
}

static void put_str(const char *msg, byte x, byte y, byte color) {
    byte i = 0;
    while (msg[i] != 0) {
	put_char(msg[i++], x++, y, color);
    }
}

static void put_row(char symbol, byte x, byte y, byte color, byte count) {
    while (count-- > 0) {
	put_char(symbol, x++, y, color);
    }
}

static byte err;
static void error_str(const char *msg) {
    put_row(' ', 0, err, 0, BORDER);
    put_str(msg, 0, err, 7);
    if (++err >= 24) err = 0;
    put_row('=', 0, err, 5, BORDER);
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

static void wait_for_button(void) {
    do { control(); } while (!dir);
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
    collision = 0;
    pos = 127;
    dir = 0;
    err = 0;
}

static void wait_vblank(void) {
    while (!vblank) { }
    vblank = 0;
}

static void vblank_delay(word ticks) {
    for (word i = 0; i < ticks; i++) { if (vblank) break; }
}

static word ticks;
static void crash_sound_vblank(int8 step) {
    while (!vblank) {
	out_fe(0x10);
	vblank_delay(ticks);
	out_fe(0x0);
	vblank_delay(ticks);
	ticks += step;
    }
    vblank = 0;
}

static byte counter;
static void jerk_vblank(void) {
    static const byte jerk_color[] = { 0, 1, 5, 7 };
    byte c = 0, s = 0;
    word i = 0, j = 0;
    word period = (224 - counter) >> 1;
    word width = (counter - 192) << 3;
    while (!vblank) {
	out_fe(jerk_color[c] | s);
	if (i == width) {
	    c = (c + 1) & 3;
	    i = 0;
	}
	if (j == period) {
	    s ^= 0x10;
	    j = 0;
	}
	j++;
	i++;
    }
    vblank = 0;
    out_fe(0);
}

static void game_vblank(void) {
    if (counter < 192) {
	wait_vblank();
    }
    else {
	jerk_vblank();
    }
}

static byte level, lives;
static const byte *pattern;
static const byte *segment;

static void next_pattern(byte inc);

struct Level {
    byte *ptr;
    word size;
    const char *msg;
};

static void next_level(byte is_ending) {
    if (counter == 0) {
	if (is_ending) counter = 192;
    }
    else if (counter++ > 224) {
	next_pattern(1);
    }
}

static void clear_row(byte y) {
    word addr = map_y[y] + 11;
    for (byte x = 0; x < WIDTH; x++) {
	BYTE(addr++) = 0;
    }
}

static void hail_stone(short x, short y) {
    x = x & 0x7f;
    if (x < 80 && 0 <= y && y < 192) {
	slow_pixel(88 + x, y);
    }
}

static void blizzard(void) {
    byte rows = 0;
    short height = ticks;
    while (height >= 0 && rows < 32) {
	short x = ticks + hail_offset[rows];
	hail_stone(x - 1, height - 1);
	hail_stone(x, height);
	height -= 6;
	rows++;
    }
    next_level(height > 192);
    ticks++;
}

static const struct Level level_list[] = {
    { level_snow, sizeof(level_snow), " GO GO GO" },
    { level_path, sizeof(level_path), " SPLENDID" },
    { (byte *) blizzard, 0, " BLIZZARD" },
    { level_trap, sizeof(level_trap), "  A TRAP" },
    { level_diam, sizeof(level_diam), " DIAMONDS" },
};

static void display_title(byte dx, byte dy);

static void put_skii_mask(byte dx, byte dy) {
    byte i = 0;
    dy = dy << 3;
    for (byte y = dy; y < dy + 6; y++) {
	for (byte x = dx; x < dx + 3; x++) {
	    BYTE(map_y[y] + x) = skii_mask[i++];
	}
    }
    for (byte x = dx; x < dx + 3; x++) {
	BYTE(0x5800 + (dy << 2) + x) = (x == dx) ? 5 : 0x45;
    }
}

static void show_life(void) {
    for (byte i = 0; i < lives; i++) {
	put_skii_mask(22, 2 + i);
    }
}

static void finish_game(void) {
    clear_screen();
    put_str("Congratulations!", 8, 4, 5);
    put_str("GAME COMPLETE", 9, 20, 5);
    put_str("You are now", 4, 9, 5);
    put_str("champion", 20, 15, 5);
    display_title(4, 10);
    wait_for_button();
    main();
}

static void end_game(void) {
    clear_screen();
    put_str("GAME OVER", 11, 11, 5);
    wait_for_button();
    main();
}

static void take_life(void) {
    if (lives-- == 0) {
	end_game();
    }
}

static void scroll(void);
static void advance(void);
static void (*callback)(void);

static void next_pattern(byte inc) {
    counter = 0;
    level += inc;
    if (SIZE(level_list) == level) finish_game();
    const struct Level *next = level_list + level;
    if (next->size > 0) {
	callback = scroll;
	pattern = next->ptr;
	segment = next->ptr + next->size;
	advance();
    }
    else {
	callback = (void(*)(void)) next->ptr;
	ticks = 0;
    }
    error_str(next->msg);
}

static void advance(void) {
    while (segment > pattern) {
	byte diff = segment[-2];
	if (diff <= counter) {
	    counter -= diff;
	    segment -= 3;
	}
	else {
	    break;
	}
    }
    if (counter > 224) {
	next_pattern(1);
    }
}

static void scroll(void) {
    byte y = counter;
    const byte *ptr = segment;
    while (y < 192 && ptr[0] > 0) {
	BYTE(map_y[y] + ptr[0]) = ptr[2];
	y += ptr[1];
	ptr += 3;
    }
    counter++;
    advance();
}

static void draw_ski(byte x, byte y, byte angle) {
    int8 dx = rotate[angle + 0];
    int8 dy = rotate[angle + 1];
    x -= dx;
    y -= dy;
    for (byte i = 0; i < 4; i++) {
	slow_pixel(x, y);
	x += dx;
	y += dy;
    }
}

static void death_loop(void) {
    byte x1 = pos + 0;
    byte y1 = 163;
    byte x2 = pos + 2;
    byte y2 = 163;
    counter = 0;
    ticks = 100;
    for (;;) {
	byte angle1 = counter & 0xe;
	byte angle2 = 14 - angle1;
	draw_ski(x1, y1, angle1);
	draw_ski(x2, y2, angle2);
	if (counter > 16) break;
	crash_sound_vblank(20);

	draw_ski(x1, y1, angle1);
	draw_ski(x2, y2, angle2);
	x1--; x2++;
	if (counter & 1) {
	    y1 -= dir;
	    y2 += dir;
	}
	counter++;
    }
    while (counter++ < 32) {
	wait_vblank();
    }
}

static void game_loop(void) {
    wait_vblank();
    next_pattern(0);
    for (;;) {
	control();
	callback();
	draw_player(1);
	game_vblank();
	draw_player(0);
	if (collision) {
	    error_str(" ACCIDENT");
	    death_loop();
	    take_life();
	    break;
	}
    }
}

static void prepare(void) {
    for (byte y = 0; y < 192; y++) {
	byte f = ((y & 7) << 3) | ((y >> 3) & 7) | (y & 0xC0);
	map_y[y] = 0x4000 + (f << 5);
    }
}

static void display_title(byte dx, byte dy) {
    word i = 0, j = 0;
    dy = dy << 3;
    for (byte y = dy; y < dy + 40; y++) {
	for (byte x = dx; x < dx + 24; x++) {
	    BYTE(map_y[y] + x) = title[i++];
	}
    }
    for (byte y = dy; y < dy + 40; y += 8) {
	for (byte x = dx; x < dx + 24; x++) {
	    BYTE(0x5800 + (y << 2) + x) = title_color[j++];
	}
    }
}

static void delay(word loops) {
    for (word i = 0; i < loops; i++) { }
}

static void ice_castle(void) {
    word decay = 9;
    word index = 0;
    word duration = 0;
    word period = music[index];

    while ((in_fe(0xfe) & 0x6) == 0x6) {
	if (period > 0) {
	    out_fe(0x10);
	    delay(period - duration);
	    out_fe(0x00);
	    delay(period + duration);
	}
	if (duration >= decay) {
	    period = 0;
	}
	if (duration >= music[index + 1]) {
	    index = music[index + 3] == 0 ? 0 : index + 2;
	    decay = music[index + 1] == 6 ? 3 : 9;
	    period = music[index];
	    duration = 0;
	}
	if (vblank) {
	    duration++;
	    vblank = 0;
	}
    }
}

static void reset_variables(void) {
    init_variables();
    level = 0;
    lives = 5;
}

void main(void) {
    __asm__("ld sp, #0xFDFC");

    setup_irq();
    prepare();

    clear_screen();
    reset_variables();
    display_title(4, 8);

    put_str("Press Z or X", 10, 16, 5);

    ice_castle();

    for (;;) {
	clear_screen();
	init_variables();
	track_border();
	track_color();
	show_life();

	game_loop();
    }
}

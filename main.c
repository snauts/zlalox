#include "main.h"
#include "data.h"

#include "level.h"

#define LIVES	5
#define WIDTH	10
#define BORDER	10

#ifdef ZXS
#define PROMPT			"Press Z or X"
#define SETUP_SP()		__asm__("ld sp, #0xFDFC")
#define READ_KEYS()		in_fe(0xfe)
#define IRQ_BASE		0xfe00
#define KEY_LEFT		0x02
#define KEY_RIGHT		0x04
#define TITLE_BUF		title
#define DENSITY			0
#define PIXEL_MAP		shift_R
#define PLAYER_ADDR		0x5180
#define LINE_INC		0x100
#define is_vsync()		vblank
#endif
#ifdef CPC
#define PROMPT			"Press X or C"
#define SETUP_SP()		__asm__("ld sp, #0xa000")
#define READ_KEYS()		cpc_keys()
#define IRQ_BASE		0xa200
#define KEY_LEFT		0x80
#define KEY_RIGHT		0x40
#define TITLE_BUF		title_cpc
#define DENSITY			1
#define PIXEL_MAP		pixel_map
#define PLAYER_ADDR		0xCE40
#define LINE_INC		0x800
#endif

#define POS_SHIFT (3 - DENSITY)
#define PIXEL_MASK (7 >> DENSITY)
#define SHIFT_PIXEL(x) ((x) << DENSITY)

#define KEY_BOTH (KEY_LEFT | KEY_RIGHT)

void main(void);

static volatile byte vblank;
static void interrupt(void) __naked {
#ifdef ZXS
    __asm__("di");
    __asm__("push af");
    __asm__("ld a, #1");
    __asm__("ld (_vblank), a");
    __asm__("pop af");
    __asm__("ei");
#endif
    __asm__("reti");
}

#ifdef CPC
static void gate_array(byte reg) {
    __asm__("ld bc, #0x7f00");
    __asm__("out (c), a"); reg;
}
static byte cpc_keys(void) __naked {
    __asm__("ld bc, #0xf782");
    __asm__("out (c), c");
    __asm__("ld bc, #0xf40e");
    __asm__("out (c), c");
    __asm__("ld bc, #0xf6c0");
    __asm__("out (c), c");
    __asm__("ld bc, #0xf600");
    __asm__("out (c), c");
    __asm__("ld bc, #0xf792");
    __asm__("out (c), c");
    __asm__("ld bc, #0xf647");
    __asm__("out (c), c");
    __asm__("ld b, #0xf4");
    __asm__("in a, (c)");
    __asm__("ld bc, #0xf782");
    __asm__("out (c), c");
    __asm__("ld bc, #0xf600");
    __asm__("out (c), c");
    __asm__("ret");
}
static byte is_vsync(void) __naked {
    __asm__("ld b, #0xf5");
    __asm__("in a, (c)");
    __asm__("and a, #1");
    __asm__("ret");
}
static byte cpc_psg(byte reg, byte val) __naked {
    __asm__("ld b, #0xf4");
    __asm__("ld c, a"); reg;
    __asm__("out (c), c");
    __asm__("ld bc, #0xf6c0");
    __asm__("out (c), c");
    __asm__("ld bc, #0xf600");
    __asm__("out (c), c");
    __asm__("ld bc, #0xf680");
    __asm__("out (c), c");
    __asm__("ld b, #0xf4");
    __asm__("ld c, l"); val;
    __asm__("out (c), c");
    __asm__("ld bc, #0xf600");
    __asm__("out (c), c");
    __asm__("ret");
}
#endif

static void memset(word addr, byte data, word len) {
    while (len-- > 0) { BYTE(addr++) = data; }
}

static void __sdcc_call_hl(void) __naked {
    __asm__("jp (hl)");
}

static void setup_irq(byte base) {
    __asm__("di");
    __asm__("ld i, a"); base;
    __asm__("im 2");
    __asm__("ei");
}

static void setup_sys(void) {
    BYTE(IRQ_BASE - 3) = 0xc3;
    WORD(IRQ_BASE - 2) = ADDR(&interrupt);
    memset(IRQ_BASE, (byte) ((IRQ_BASE >> 8) - 1), 0x101);
    setup_irq(IRQ_BASE >> 8);

#ifdef CPC
    __asm__("ld bc, #0xbc0c");
    __asm__("out (c), c");
    __asm__("ld bc, #0xbd33");
    __asm__("out (c), c");

    __asm__("ld bc, #0xbc0d");
    __asm__("out (c), c");
    __asm__("ld bc, #0xbdd4");
    __asm__("out (c), c");

    cpc_psg(7, 0xBC);
    cpc_psg(8, 0x00);
    for (byte i = 0; i < SIZE(gate_array_init); i++) {
	gate_array(gate_array_init[i]);
    }
#endif
}

#ifdef ZXS
static void out_fe(byte data) {
    __asm__("out (#0xfe), a"); data;
}

static byte in_fe(byte a) __naked {
    __asm__("in a, (#0xfe)"); a;
    __asm__("ret");
}
#endif

static word map_y[192];

static void slow_pixel(byte x, byte y) {
    BYTE(map_y[y] + (x >> POS_SHIFT)) ^= PIXEL_MAP[x & PIXEL_MASK];
}

static void clear_screen(void) {
#ifdef ZXS
    memset(0x4000, 0x00, 0x1B00);
#endif
#ifdef CPC
    memset(0xC000, 0x00, 0x4000);
#endif
}

static void track_color(void) {
#ifdef ZXS
    memset(0x5800, 0x01, 0x300);
    memset(0x5880, 0x05, 0x260);
    memset(0x5900, 0x07, 0x1C0);
    memset(0x5980, 0x47, 0x120);
#endif
}

static void track_border(void) {
#ifdef ZXS
    for (word addr = 0x4000; addr < 0x5800; addr += 0x20) {
	BYTE(addr + 31 - BORDER) = 0xF0;
	BYTE(addr + BORDER) = 0x0F;
    }
#endif
#ifdef CPC
    for (byte y = 0; y < 192; y++) {
	word addr = map_y[y];
	addr += SHIFT_PIXEL(BORDER);
	BYTE(addr++) = 0x03;
	BYTE(addr++) = 0xF3;
	addr += SHIFT_PIXEL(WIDTH);
	BYTE(addr++) = 0xFC;
	BYTE(addr++) = 0x0C;
    }
#endif
}

static void put_char(char symbol, byte x, byte y, byte color) {
    y = y << 3;
#ifdef ZXS
    word addr = 0x3C00 + (symbol << 3);
    for (byte i = 0; i < 8; i++) {
	BYTE(map_y[y + i] + x) = BYTE(addr + i);
    }
    BYTE(0x5800 + (y << 2) + x) = color;
#endif
#ifdef CPC
    color;
    x = x << 1;
    word glyph = symbol - 0x20;
    const byte *addr = font_cpc + (glyph << 4);
    for (byte i = 0; i < 16; i += 2) {
	word pos = map_y[y++] + x;
	BYTE(pos + 0) = addr[i + 0];
	BYTE(pos + 1) = addr[i + 1];
    }
#endif
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
}

static char to_hex(byte digit) {
    return (digit < 10) ? '0' + digit : 'A' + digit - 10;
}

static void put_num(word num, byte x, byte y, byte color) {
    char msg[] = "0x0000";
    for (byte i = 5; i >= 2; i--) {
	msg[i] = to_hex(num & 0xf);
	num = num >> 4;
    }
    put_str(msg, x, y, color);
}

static byte pos;
static int8 dir;
static void control(void) {
    byte port = READ_KEYS();
    dir = 0;
    if ((port & KEY_LEFT) == 0) dir--;
    if ((port & KEY_RIGHT) == 0) dir++;
    pos += dir;
}

static void wait_for_button(void) {
    do { control(); } while (!dir);
}

static byte detect, collision;
static void draw_straight(void) {
    word addr = PLAYER_ADDR + (pos >> POS_SHIFT);
    word data = pixels[pos & PIXEL_MASK];
    for (byte n = 0; n < 5; n++) {
	word screen = WORD(addr);
	if (detect && (screen & data)) collision = 1;
	WORD(addr) = screen ^ data;
	addr += LINE_INC;
    }
}

static void draw_side(byte x, word data) {
    data += (x & PIXEL_MASK) << 3;
    word addr = PLAYER_ADDR + LINE_INC + (x >> POS_SHIFT);
    for (byte n = 0; n < 4; n++) {
	word screen = WORD(addr);
	word sprite = WORD(data);
	if (detect && (screen & sprite)) collision = 1;
	WORD(addr) = screen ^ sprite;
	addr += LINE_INC;
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
}

static void wait_vblank(void) {
    while (!is_vsync()) { }
    vblank = 0;
}

#ifdef ZXS
static void vblank_delay(word ticks) {
    for (word i = 0; i < ticks; i++) { if (is_vsync()) break; }
}
#endif

static word ticks;
static void crash_sound_vblank(void) {
#ifdef ZXS
    while (!is_vsync()) {
	out_fe(0x10);
	vblank_delay(ticks);
	out_fe(0x0);
	vblank_delay(ticks);
	ticks += 20;
    }
    vblank = 0;
#endif
#ifdef CPC
    cpc_psg(0, ticks & 0xff);
    cpc_psg(1, ticks >> 8);
    ticks += 64;
    wait_vblank();
#endif
}

static byte counter;

#ifdef CPC
static void cpc_level_finish_sound(void) {
    static word freq;
    byte step = counter - 192;
    if (step == 0) {
	freq = 225;
	cpc_psg(8, 0x0F);
    }
    cpc_psg(0, freq & 0xff);
    cpc_psg(1, freq >> 8);
    freq -= 6;
    if (step >= 32) {
	cpc_psg(8, 0x10);
    }
}
#endif

static void jerk_vblank(void) {
    byte c = 0, s = 0;
    word i = 0, j = 0;
    word period = (224 - counter) >> 1;
    word width = (counter - 192) << 3;
#ifdef CPC
    cpc_level_finish_sound();
#endif
    while (!is_vsync()) {
#ifdef ZXS
	static const byte jerk_color[] = { 0, 1, 5, 7 };
	out_fe(jerk_color[c] | s);
#endif
#ifdef CPC
	static const byte jerk_color[] = { 0x54, 0x53, 0x57, 0x4B };
	gate_array(0x10);
	gate_array(jerk_color[c]);
#endif
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
#ifdef ZXS
    out_fe(0);
#endif
#ifdef CPC
    gate_array(0x10);
    gate_array(0x54);
#endif
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

static void set_row(short y, byte data) {
    if (0 <= y && y < 192) {
	word addr = map_y[y] + (11 << DENSITY);
	for (byte x = 0; x < (WIDTH << DENSITY); x++) {
	    BYTE(addr++) = data;
	}
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

static void snower(void) {
    byte rows = 0;
    short height = ticks;
    while (height >= 0 && rows < 24) {
	byte x = snow_offset[rows];
	byte offset = x + height;
	hail_stone(x + swirl[offset & 0x1f], height);
	offset--;
	hail_stone(x + swirl[offset & 0x1f], height - 1);
	height -= 8;
	rows++;
    }
    next_level(height > 192);
    ticks++;
}

static void draw_worm(byte x, byte offset, byte cut) {
    x += swirl[(ticks + offset) & 0x1f];
    short y = ticks - cut;
    hail_stone(x - 1, y);
    hail_stone(x + 1, y);
    hail_stone(x, y);
}

static byte steer(byte x, byte target) {
    if (x > target) x--;
    if (x < target) x++;
    return x;
}

static void ice_worm(void) {
    static byte worm1, worm2;
    if (ticks == 0) {
	worm1 = 25;
	worm2 = 65;
    }
    draw_worm(worm1, 12, 0);
    draw_worm(worm2, 24, 4);
    set_row(ticks - 32, 0);
    if (ticks & 1) {
	byte p = pos - 88;
	if (p < 50) {
	    worm1 = steer(worm1, p);
	    worm2 = steer(worm2, 70);
	}
	else {
	    worm1 = steer(worm1, 10);
	    worm2 = steer(worm2, p);
	}
    }
    next_level(ticks > 256);
    ticks++;
}

static void draw_wall(byte offset, byte exit) {
    short y = ticks - offset;
    set_row(y, 0xff);
    set_row(y - 4, 0);
    if (y >= 96 && y < 192) {
	byte i = y < 104 ? (y & 7) : 7;
	word addr = map_y[y] + SHIFT_PIXEL(exit);
#if DENSITY == 0
	BYTE(addr + 1) = gate_R[i];
	BYTE(addr + 0) = gate_L[i];
#else
	if (i < 4) {
	    BYTE(addr + 2) = gate_R[i];
	    BYTE(addr + 1) = gate_L[i];
	}
	else {
	    BYTE(addr + 3) = gate_R[i - 4];
	    BYTE(addr + 2) = 0;
	    BYTE(addr + 1) = 0;
	    BYTE(addr + 0) = gate_L[i - 4];
	}
#endif
    }
}

static void gates(void) {
    draw_wall(  0, 18);
    draw_wall( 48, 14);
    draw_wall( 96, 17);
    draw_wall(144, 12);
    draw_wall(192, 18);
    next_level(ticks > (192 + 192 + 4));
    ticks++;
}

static void mover_gate(byte *addr, byte i) {
    for (byte j = 0; j <= SHIFT_PIXEL(2); j++) {
	byte data = 0;
	if (j == 0) {
	    data = gate_L[PIXEL_MASK - i];
	}
	else if (j == SHIFT_PIXEL(2)) {
	    data = gate_R[i];
	}
	*(addr++) = data;
    }
}

static void draw_mover(word offset, byte exit) {
    short y = ticks - offset;
    set_row(y, 0xff);
    set_row(y - 4, 0);

    if (y < 192) {
	word addr = map_y[y];
	byte x, i = ((y >> 1) & PIXEL_MASK);
	byte move = (ticks >> (4 - DENSITY)) + exit;
	byte offset = (move & ((8 << DENSITY) - 1));

	if ((move & (8 << DENSITY)) == 0) {
	    x = SHIFT_PIXEL(11) + offset;
	}
	else {
	    x = SHIFT_PIXEL(18) - offset + DENSITY;
	    i = PIXEL_MASK - i;
	}
	mover_gate((byte *) (addr + x), i);
    }
}

static void movers(void) {
    draw_mover(  0,  0);
    draw_mover( 64,  6);
    draw_mover(128, 14);
    draw_mover(192, 18);
    draw_mover(256,  8);
    next_level(ticks > 256 + 192 + 16);
    ticks++;
}

static void draw_corner(short y, byte level) {
    for (byte i = 0; i < 9; i++) {
	if (y >= 0 && y < 192) {
	    word addr = map_y[y];
	    byte b1 = 0xff, b2 = 0xff;
#if DENSITY == 0
	    if (i > 0) {
		b1 = gate_L[i - 1];
		b2 = gate_R[i - 1];
	    }
	    BYTE(addr + SHIFT_PIXEL(level + 11)) = b1;
	    BYTE(addr + SHIFT_PIXEL(20 - level) + DENSITY) = b2;
#else
	    if (i > 4) {
		b1 = gate_L[i - 5];
		b2 = gate_R[i - 5];
	    }
	    BYTE(addr + SHIFT_PIXEL(level + 11)) = b1;
	    BYTE(addr + SHIFT_PIXEL(20 - level) + DENSITY) = b2;

	    b1 = b2 = i > 0 ? 0x00 : 0xff;

	    if (0 < i && i < 5) {
		b1 = gate_L[i - 1];
		b2 = gate_R[i - 1];
	    }
	    BYTE(addr + SHIFT_PIXEL(level + 11) + DENSITY) = b1;
	    BYTE(addr + SHIFT_PIXEL(20 - level)) = b2;
#endif
	}
	y++;
    }
}

static void draw_crown(void) {
    short y = ticks - 832;
    byte step = (2 << DENSITY);
    for (byte i = 0; i < SIZE(crown); i += step) {
	if (y >= 0 && y < 192) {
	    word addr = map_y[y];
	    for (byte x = 0; x < step; x++) {
		BYTE(addr + x + SHIFT_PIXEL(15)) = crown[i + x];
	    }
	}
	y++;
    }
}

static void finish_game(void);
static void castle(void) {
    if (ticks < 952) {
	byte level = 0;
	word y = ticks;
	while (y >= 200) {
	    y = y - 200;
	    level++;
	}
	draw_corner(y - 8, level);
	draw_crown();
    }
    else if (ticks == 976 || ticks == 1008 || ticks == 1040) {
	counter = 192;
    }
    else if (counter > 0 && counter <= 224) {
	counter++;
    }
    if (ticks == 1080) {
	finish_game();
    }
    ticks++;
}

static void release_bomb(short t, byte x) {
    byte dx = 2;
    short y = ticks - t;
    if (y > 128) dx = y - 128;
    byte hx = dx >> 1;
    hail_stone(x + dx, y - 2);
    hail_stone(x - dx, y - 2);
    hail_stone(x + hx, y - 1);
    hail_stone(x - hx, y - 1);
    hail_stone(x, y);
}

static void bombs(void) {
    release_bomb(  0, 40);
    release_bomb(  1, 40);
    release_bomb( 32, 20);
    release_bomb( 33, 20);
    release_bomb( 64, 60);
    release_bomb( 65, 60);
    release_bomb( 96, 30);
    release_bomb( 97, 30);
    release_bomb(128, 50);
    release_bomb(129, 50);
    next_level(ticks > (192 + 144));
    ticks++;
}

static const struct Level level_list[] = {
    { level_snow, sizeof(level_snow), " GO.GO.GO" },
    { level_path, sizeof(level_path), " (BURROW)" },
    { (byte *) ice_worm, 0, " ICE-WORM" },
    { level_tetr, sizeof(level_tetr), " =TETRIS=" },
    { (byte *) snower, 0, " +SNOWER+" },
    { level_trap, sizeof(level_trap), " -<TRAP>-" },
    { (byte *) movers, 0, " /MOVERS/" },
    { level_inva, sizeof(level_inva), " CHAMBERS" },
    { (byte *) gates, 0, " ^[GATE]^" },
    { level_berg, sizeof(level_berg), " ICE-BERG" },
    { (byte *) bombs, 0, " v*BOMB*v" },
    { level_maze, sizeof(level_maze), " {-MAZE-}" },
    { (byte *) blizzard, 0, " BLIZZARD" },
    { level_diam, sizeof(level_diam), " DIAMONDS" },
    { (byte *) castle, 0, " !CASTLE!" },
};

static void display_title(byte dx, byte dy);

static void put_skii_mask(byte dx, byte dy) {
    byte i = 0;
    dy = dy << 3;
    dx = SHIFT_PIXEL(dx);
    for (byte y = dy; y < dy + 6; y++) {
	for (byte x = dx; x < dx + SHIFT_PIXEL(3); x++) {
	    BYTE(map_y[y] + x) = skii_mask[i++];
	}
    }
#ifdef ZXS
    for (byte x = dx; x < dx + 3; x++) {
	BYTE(0x5800 + (dy << 2) + x) = (x == dx) ? 5 : 0x45;
    }
#endif
}

static void show_life(void) {
    for (byte i = 0; i < lives; i++) {
	put_skii_mask(22, 2 + i);
    }
}

static void ice_castle(void);
static void finish_game(void) {
    clear_screen();
    put_str("Congratulations!", 8, 4, 5);
    put_str("GAME COMPLETE", 9, 20, 5);
    put_str("You are now", 4, 9, 5);
    put_str("champion", 20, 15, 5);
    display_title(4, 10);
    ice_castle();
    main();
}

static void end_game(void) {
    clear_screen();
    put_str("GAME OVER", 11 + DENSITY * 4, 11, 5);
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
    if (level == err) error_str(next->msg);
}

static void advance(void) {
    while (segment > pattern) {
	byte diff = segment[-1];
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
    const byte *ptr = segment;
    byte x = *ptr;
    byte y = counter;
    byte *addr;
    while (y < 192 && x > 0) {
	addr = (byte *) map_y[y] + x;
	*addr = *(++ptr);
	y += *(++ptr);
	x = *(++ptr);
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

#define L_BUMP (8 * (WIDTH + 1))
#define R_BUMP (8 * (WIDTH + BORDER + 1))

static void death_loop(void) {
    byte x1 = pos + 0;
    byte y1 = 163;
    signed char d1 = -1;
    byte x2 = pos + 2;
    byte y2 = 163;
    signed char d2 = 1;
    counter = 0;
#ifdef ZXS
    ticks = 100;
#endif
#ifdef CPC
    ticks = 225;
    cpc_psg(8, 0x0F);
#endif
    for (;;) {
	byte angle1 = counter & 0xe;
	byte angle2 = 14 - angle1;
	draw_ski(x1, y1, angle1);
	draw_ski(x2, y2, angle2);
	if (counter > 16) break;
	crash_sound_vblank();

	draw_ski(x1, y1, angle1);
	draw_ski(x2, y2, angle2);
	if (x1 < L_BUMP) d1 = +1;
	if (x2 > R_BUMP) d2 = -1;
	x1 += d1;
	x2 += d2;
	if (counter & 1) {
	    y1 -= dir;
	    y2 += dir;
	}
	counter++;
    }
#ifdef CPC
    cpc_psg(8, 0x10);
#endif
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
	    death_loop();
	    take_life();
	    break;
	}
    }
}

#ifdef CPC
static word mul80(word x) {
    return (x << 6) + (x << 4);
}
#endif

static void prepare(void) {
    for (byte y = 0; y < 192; y++) {
#ifdef ZXS
	byte f = ((y & 7) << 3) | ((y >> 3) & 7) | (y & 0xC0);
	map_y[y] = 0x4000 + (f << 5);
#endif
#ifdef CPC
	word f = ((y & 7) << 11) | mul80(y >> 3);
	map_y[y] = 0xC000 + f;
#endif
    }
}

static void display_title(byte dx, byte dy) {
    word i = 0, j = 0;
    dy = dy << 3;
    dx = SHIFT_PIXEL(dx);
    for (byte y = dy; y < dy + 40; y++) {
	for (byte x = dx; x < dx + SHIFT_PIXEL(24); x++) {
	    BYTE(map_y[y] + x) = TITLE_BUF[i++];
	}
    }
#ifdef ZXS
    for (byte y = dy; y < dy + 40; y += 8) {
	for (byte x = dx; x < dx + 24; x++) {
	    BYTE(0x5800 + (y << 2) + x) = title_color[j++];
	}
    }
#endif
}

static void delay(word loops) {
    for (word i = 0; i < loops; i++) { }
}

static byte is_vblank_start(void) {
#ifdef ZXS
    byte ret = vblank;
    if (ret) vblank = 0;
    return ret;
#endif
#ifdef CPC
    byte new = is_vsync();
    byte old = vblank;
    vblank = new;
    return new && !old;
#endif
}

#ifdef CPC
static void cpc_play_note(word frequency, byte volume, byte channel) {
    if (frequency > 0) {
	byte offset = (channel << 1);
	cpc_psg(0 + offset, frequency & 0xff);
	cpc_psg(1 + offset, frequency >> 8);
    }
    cpc_psg(8 + channel, volume);
}
#define PLAY_NOTE(f, v, c) cpc_play_note(f, v, c)
#else
#define PLAY_NOTE(f, v, c)
#endif

static void ice_castle(void) {
    const byte *tune = music;

    byte decay = 9;
    byte octave = 0;
    byte duration = 0;
    word period = tune[0];

    PLAY_NOTE(period, 0xf, 0);
    while ((READ_KEYS() & KEY_BOTH) == KEY_BOTH) {
#ifdef ZXS
	if (period > 0) {
	    out_fe(0x10);
	    delay(period);
	    out_fe(0x00);
	    delay(period);
	}
#endif
	if (is_vblank_start()) {
	    duration++;
	    if (duration >= decay) {
		PLAY_NOTE(0, 0x10, 0);
		period = 0;
	    }
	    if (duration >= tune[1]) {
		tune += 2;
		if (tune[1] == 0) {
		    octave = (octave + 3) & 3;
		    tune = music;
		}
		decay = tune[1] == L16 ? 3 : 9;
		period = tune[0] << octave;
		PLAY_NOTE(period, 0xf, 0);
		duration = 0;
	    }
	}
    }
}

static void reset_variables(void) {
    init_variables();
    lives = LIVES;
    level = 0;
    err = 0;
}

static void start_screen(void) {
    put_str(PROMPT, 10, 16, 5);
    display_title(4, 8);
    wait_for_button();
}

static void clear_rectangle(byte x1, byte y1, byte x2, byte y2) {
    for (byte y = y1; y < y2; y++) {
	byte *addr = (byte *) (map_y[y] + x1);
	for (byte x = x1; x < x2; x++) {
	    *(addr++) = 0;
	}
    }
}

static void clear_track(void) {
    clear_rectangle(SHIFT_PIXEL(11), 0, SHIFT_PIXEL(21), 192);
    clear_rectangle(SHIFT_PIXEL(22), (lives + 2) << 3,
		    SHIFT_PIXEL(25), (LIVES + 2) << 3);
}

static void message_bar(void) {
    put_row('-', 0, SIZE(level_list), 5, BORDER);
}

void main(void) {
    SETUP_SP();

    setup_sys();
    prepare();

    clear_screen();
    reset_variables();
    start_screen();

    clear_screen();
    track_color();
    message_bar();
    show_life();

    for (;;) {
	init_variables();
	clear_track();
	track_border();

	game_loop();
    }
}

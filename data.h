#ifdef ZXS
#define D4  145		// 293.7
#define E4  128		// 329.6
#define F4s 113		// 369.9
#define G4  108		// 392.0
#define G4s 100		// 415.3
#define A4  95		// 440.0
#define B4  83		// 493.9
#define C5s 73		// 554.4
#define D5  68		// 587.3
#endif

#ifdef CPC
#define F3  358		// 174.6
#define F3s 338		// 185.0
#define G3s 301		// 207.6
#define A3  284		// 220.0
#define B3  254		// 246.9
#define C4s 266		// 277.1
#define D4  213		// 293.7
#define E4  190		// 329.6
#define F4  179		// 349.2
#define F4s 169		// 369.9
#define G4  159		// 392.0
#define G4s 150		// 415.3
#define A4  142		// 440.0
#define A4s 134		// 466.1
#define B4  127		// 493.9
#define C5s 113		// 554.4
#define D5  106		// 587.3
#define D5s 100		// 622.2
#define F5  89		// 698.4
#define E5  95		// 659.2
#define A5  71		// 880.0
#define A5s 67		// 932.3
#define B5  63		// 987.7

#define L2  48
#define L4  24
#define L8D 18
#define L8  12
#define L16  6

static const byte music[] = {
    A4,  L8,  D4,  L8,  F4s, L4, F4s, L8, E4,  L4, E4, L8,
    G4,  L8,  E4,  L8,  G4,  L8, G4,  L8, F4s, L8, A4, L8, A4, L4,
    A4,  L8 , D4,  L8,  F4s, L4, F4s, L8, E4,  L4, E4, L8,
    G4,  L8 , E4,  L8,  E4,  L8, E4,  L8, D4,  L8, D4, L8, D4, L4,

    D5,  L8D, C5s, L16, C5s, L8, A4,  L8, B4,  L8, B4, L8, B4, L4,
    C5s, L8D, B4,  L16, B4,  L8, G4s, L8, A4,  L8, A4, L8, A4, L4,
    D5,  L8D, C5s, L16, C5s, L8, A4,  L8, B4,  L8, B4, L8, B4, L4,
    C5s, L8D, B4,  L16, B4,  L8, G4s, L8, A4,  L8, A4, L8, A4, L4,
    0, 0
};

//	  A5-
//	  G5
//	--F5--
//	  E5
//	--D5--
//	  C5
//	--B4--
//	  A4
//	--G4--
//	  F4
//	--E4--
//	  D4
//	  C4-

// 𝅘𝅥 - L4, 𝅘𝅥𝅮 - L8, 𝅘𝅥𝅯 - L16

static const word chord1[] = {
    F4s, L4,  A4,  L8,  A3,  L8,  E4,  L4,  G4,  L8, A3,  L8,
    E4,  L8,  C4s, L8,  E4,  L8,  C4s, L8,  D4,  L8, F4s, L8, D4,  L8, F4s, L8,
    F4s, L4,  A4,  L8,  A3,  L8,  E4,  L4,  G4,  L8, A3,  L8,
    E4,  L8,  C4s, L8,  E4,  L8,  C4s, L8,  D4,  L8, A3,  L8, A3,  L8, 0,   L8,
    A3,  L8,  D4,  L8,  A3,  L8,  D4,  L8,  B3,  L8, D4,  L8, B3,  L8, D4,  L8,
    F3,  L8,  B3,  L8,  G3s, L8,  B3,  L8,  F3s, L8, D4,  L8, L8,  A3, L8,  D4,
    A3,  L8,  D4,  L8,  A3,  L8,  D4,  L8,  B3,  L8, D4,  L8, L8,  B3, L8,  D4,
    F3,  L8,  B3,  L8,  G3s, L8,  B3,  L8,  A3,  L8, A3,  L8, A3,  L4,
    0, 0
};
static const word chord2[] = {
    B5,  L2, A5s, L2,
    A5s, L8, F5,  L8, A5s, L8,  F5,  L8, B5,  L8,  F5,  L8, B5,  L8,  F5,  L8,
    B5,  L2, A5s, L2,
    A5s, L8, F5,  L8, A5s, L8,  F5,  L8, F5,  L8,  D5s, L8, D5s, L4,
    D5s, L4, D5s, L4, E5,  L8,  E5,  L8, E5,  L4,
    A4s, L4, A4s, L4, D5s, L4,  D5s, L4,
    D5s, L4, D5s, L4, E5,  L8,  E5,  L8, E5,  L4,
    A4s, L4, A4s, L4, D5s, L8,  D5s, L8, F5,  L8,  F4, L8,
    0, 0
};
#endif

#ifdef ZXS
static const word pixels[] = {
    0x00A0, 0x0050, 0x0028, 0x0014,
    0x000A, 0x0005, 0x8002, 0x4001
};
#endif
#ifdef CPC
static const word pixels[] = {
    0x00AA, 0x0055, 0x8822, 0x4411,
};
#endif

#ifdef ZXS
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
#endif
#ifdef CPC
static const word pixels_L[] = {
    0x00AA, 0x00AA, 0x0055, 0x8822,
    0x0055, 0x0055, 0x8822, 0x4411,
    0x8822, 0x8822, 0x4411, 0xAA00,
    0x4411, 0x4411, 0xAA00, 0x5500,
};
#endif

#ifdef ZXS
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
#endif

#ifdef CPC
static const word pixels_R[] = {
    0x8822, 0x8822, 0x0055, 0x00AA,
    0x4411, 0x4411, 0x8822, 0x0055,
    0xAA00, 0xAA00, 0x4411, 0x8822,
    0x5500, 0x5500, 0xAA00, 0x4411,
};
#endif

static const int8 rotate[] = {
    0, +1, +1, +1, +1, 0, +1, -1, 0, -1, -1, -1, -1, 0, -1, +1
};

#ifdef ZXS
static const byte title_color[] = {
    0x69, 0x69, 0x69, 0x41, 0x4D, 0x41, 0x47, 0x47,
    0x41, 0x4D, 0x48, 0x41, 0x4D, 0x41, 0x47, 0x47,
    0x41, 0x4D, 0x48, 0x41, 0x4D, 0x41, 0x4D, 0x41,

    0x45, 0x47, 0x47, 0x47, 0x45, 0x47, 0x47, 0x47,
    0x4D, 0x4F, 0x47, 0x47, 0x45, 0x47, 0x47, 0x47,
    0x4D, 0x4F, 0x47, 0x47, 0x45, 0x47, 0x4D, 0x47,

    0x41, 0x41, 0x4F, 0x47, 0x45, 0x47, 0x47, 0x47,
    0x45, 0x47, 0x4D, 0x47, 0x45, 0x47, 0x47, 0x47,
    0x45, 0x47, 0x45, 0x47, 0x41, 0x4F, 0x4F, 0x47,

    0x4D, 0x4F, 0x4F, 0x41, 0x45, 0x47, 0x41, 0x41,
    0x45, 0x47, 0x47, 0x47, 0x45, 0x47, 0x41, 0x41,
    0x45, 0x47, 0x4D, 0x47, 0x4D, 0x4F, 0x6F, 0x47,

    0x45, 0x47, 0x47, 0x47, 0x45, 0x47, 0x47, 0x47,
    0x45, 0x47, 0x45, 0x47, 0x45, 0x47, 0x47, 0x47,
    0x45, 0x47, 0x47, 0x47, 0x45, 0x47, 0x45, 0x47,
};
#endif

#ifdef ZXS
static const byte skii_mask[] = {
    0x01, 0xf8, 0xf8,
    0x01, 0xc5, 0x18,
    0x01, 0xc2, 0x18,
    0x01, 0xc0, 0x18,
    0x00, 0xe0, 0x30,
    0x00, 0x7f, 0xe0,
};

static const byte crown[] = {
    0xff, 0xff,
    0xdf, 0xfb,
    0xce, 0x73,
    0xc4, 0x23,
    0xc0, 0x03,
    0xc0, 0x03,
    0xe0, 0x07,
    0xf0, 0x0f,
};
#endif

static const byte shift_R[] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static const byte hail_offset[] = {
    0x00, 0x2F, 0x5E, 0x11, 0x4F, 0x77, 0x33, 0x67,
    0x24, 0x4F, 0x05, 0x34, 0x68, 0x08, 0x2D, 0x56,
    0x0E, 0x2E, 0x5B, 0x7D, 0x21, 0x5A, 0x08, 0x33,
    0x70, 0x12, 0x46, 0x6E, 0x29, 0x4D, 0x6D, 0x0F,
};

static const byte snow_offset[] = {
    43, 68, 20, 61, 21, 78, 50, 0,
    43, 57, 15, 30, 59, 32, 10, 30,
    70, 38, 9, 50, 10, 72, 2, 31,
};

static const int8 swirl[] = {
    -1, -2, -2, -3, -3, -4, -4, -4,
    -4, -4, -3, -3, -2, -2, -1,  0,
     1,  2,  2,  3,  3,  4,  4,  4,
     4,  4,  3,  3,  2,  2,  1,  0,
};

#ifdef ZXS
static const byte gate_L[] = {
    0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00
};
static const byte gate_R[] = {
    0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00
};
#endif
#ifdef CPC
static const byte gate_L[] = {
    0xEF, 0xCE, 0x8C, 0x08
};
static const byte gate_R[] = {
    0xF7, 0x73, 0x31, 0x10
};
#endif

#ifdef CPC
static const byte gate_array_init[] = {
    0x9D, 0x10, 0x54, 0, 0x54, 1, 0x53, 2, 0x57, 3, 0x4B
};

static const byte pixel_map[] = {
    0x88, 0x44, 0x22, 0x11,
};
#endif

static const word tens[] = {
    10000, 1000, 100, 10, 1
};

#ifdef ZXS
    static const byte jerk_color[] = { 0x00, 0x01, 0x05, 0x07 };
#endif
#ifdef CPC
    static const byte jerk_color[] = { 0x54, 0x53, 0x57, 0x4B };
#endif

#define D4  145		// 293.7
#define E4  128		// 329.6
#define F4s 113		// 369.9
#define G4  108		// 392.0
#define G4s 100		// 415.3
#define A4  95		// 440.0
#define B4  83		// 493.9
#define C5s 73		// 554.4
#define D5  68		// 587.3

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

static const word pixels[] = {
    0x00A0, 0x0050, 0x0028, 0x0014,
    0x000A, 0x0005, 0x8002, 0x4001
};

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

static const int8 rotate[] = {
    0, +1, +1, +1, +1, 0, +1, -1, 0, -1, -1, -1, -1, 0, -1, +1
};

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

static const byte skii_mask[] = {
    0x01, 0xf8, 0xf8,
    0x01, 0xc5, 0x18,
    0x01, 0xc2, 0x18,
    0x01, 0xc0, 0x18,
    0x00, 0xc0, 0x10,
    0x00, 0x7f, 0xe0,
};

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
    78, 18, 57, 42, 19, 8, 1, 73, 15, 12, 34, 79,
    44, 35, 37, 13, 38, 49, 9, 36, 60, 1, 57, 2,
};

static const int8 swirl[] = {
    -1, -2, -2, -3, -3, -4, -4, -4,
    -4, -4, -3, -3, -2, -2, -1, 0,
    1, 2, 2, 3, 3, 4, 4, 4,
    4, 4, 3, 3, 2, 2, 1, 0,
};

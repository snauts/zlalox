static unsigned char a, b, c, x;

static unsigned char rnd8(void) {
    x = x + 1;
    a = (a ^ c) ^ x;
    b = b + a;
    c = (c + (b >> 1)) ^ a;
    return c;
}

void main(void) {
    __asm__("ld sp, #0xff40");
    while (1) {
	unsigned short i;
	for (i = 0x4000; i < 0x5800; i++) {
	    * (volatile unsigned char *) i = rnd8();
	}
    }
}

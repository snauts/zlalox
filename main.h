typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

#define BYTE(addr) (* (volatile byte *) (addr))
#define WORD(addr) (* (volatile word *) (addr))
#define ADDR(obj) ((word) (obj))

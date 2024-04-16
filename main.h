typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

#define MEM(addr) (* (volatile byte *) (addr))
#define M16(addr) (* (volatile word *) (addr))
#define ADDR(obj) ((word) (obj))

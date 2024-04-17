typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

#define SIZE(array) (sizeof(array) / sizeof(*(array)))
#define BYTE(addr) (* (volatile byte *) (addr))
#define WORD(addr) (* (volatile word *) (addr))
#define ADDR(obj) ((word) (obj))

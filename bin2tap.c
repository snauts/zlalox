#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/param.h>

#ifndef ADDRESS
#define ADDRESS		0x8000
#endif

#define HDR_TYPE	3
#define HDR_NAME	4
#define HDR_LEN1	14
#define HDR_ARG1	16
#define HDR_ARG2	18
#define HDR_LEN2	21
#define HDR_FLAG	23

#define LINE_NUM	10

static unsigned char *add_header(unsigned char *buf) {
    static unsigned char header[] = {
	0x13, 0x00, 0x00, 0x00,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    memcpy(buf, header, sizeof(header));
    return buf + sizeof(header);
}

static void add_name(unsigned char *ptr, char *name) {
    memcpy(ptr + HDR_NAME, name, MIN(10, strlen(name)));
}

static void add_word(unsigned char *ptr, int offset, unsigned short word) {
    memcpy(ptr + offset, &word, sizeof(word));
}

static void put_number(char *ptr, int num) {
    sprintf(ptr, "%d", num);
    ptr[5] = '"';
}

static void put_line(unsigned char *ptr, int line, int size) {
    memset(ptr, 0, 4);
    ptr[1] = line;
    ptr[2] = size;
}

static int add_binary(char *name, unsigned char *ptr) {
    int size = 0;
    int fd = open(name, O_RDONLY);
    if (fd >= 0) {
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	read(fd, ptr, size);
	close(fd);
    }
    return size;
}

static int add_loader(unsigned char *ptr) {
    const char *loader =
	"\xD9\xB0\"7\":"
	"\xDA\xB0\"7\":"
	"\xE7\xB0\"7\":"
	"\xFB:"
	"\xEF\"\"\xAF:"
	"\xF9\xC0\xB0\"ADDR0\"";

    int size = strlen(loader);
    put_line(ptr, LINE_NUM, size);

    ptr += 4;
    strcpy(ptr, loader);
    put_number(strstr(ptr, "ADDR0"), ADDRESS);

    return 4 + size;
}

static int add_checksum(unsigned char *ptr, int amount) {
    unsigned char checksum = 0;
    for (int i = 0; i < amount; i++) {
	checksum ^= ptr[i];
    }
    ptr[amount] = checksum;
}

static void save_tap(char *name, unsigned char *tap, int size) {
    int fd = open(name, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd >= 0) {
	write(fd, tap, size);
	close(fd);
    }
}

static int fill_header(unsigned char *ptr, int size, int type) {
    add_word(ptr, HDR_LEN1, size);
    add_word(ptr, HDR_LEN2, size + 2);

    ptr[HDR_TYPE] = type;
    ptr[HDR_FLAG] = 0xff;

    add_checksum(ptr + HDR_TYPE, 17); /* header checksum */
    add_checksum(ptr + HDR_FLAG, size + 1);
    return HDR_FLAG + 1 + size + 1;
}

int main(int argc, char **argv) {
    if (argc < 3) {
	fprintf(stderr, "usage: bin2tap [in.bin] [out.tap]\n");
	return 0;
    }

    int len;

    unsigned char *tap = malloc(0x10000);
    unsigned char *ptr = tap;

    /* loader data */
    len = add_loader(add_header(ptr));

    add_name(ptr, "Liezere");
    add_word(ptr, HDR_ARG1, LINE_NUM);
    add_word(ptr, HDR_ARG2, len);
    ptr += fill_header(ptr, len, 0);

    /* program data */
    len = add_binary(argv[1], add_header(ptr));

    add_name(ptr, "program");
    add_word(ptr, HDR_ARG1, ADDRESS);
    add_word(ptr, HDR_ARG2, 0x8000);
    ptr += fill_header(ptr, len, 3);

    save_tap(argv[2], tap, ptr - tap);

    free(tap);
    return 0;
}

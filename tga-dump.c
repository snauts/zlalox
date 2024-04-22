#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define START	11
#define WIDTH	10

struct Header {
    unsigned char id;
    unsigned char color_type;
    unsigned char image_type;
    unsigned char color_map[5];
    unsigned short x, y, w, h;
    unsigned char depth;
    unsigned char desc;
};

static void save_level(struct Header *header, unsigned char *buf);
static void save_bitmap(struct Header *header, unsigned char *buf);
static void save_font_cpc(struct Header *header, unsigned char *buf);
static void save_bitmap_cpc(struct Header *header, unsigned char *buf);

static unsigned char consume_pixels(unsigned char *buf) {
    unsigned char ret = 0;
    for (int i = 0; i < 8; i++) {
	ret = ret << 1;
	ret |= (buf[i] >> 7) & 1;
    }
    return ret;
}

static unsigned char consume_pixels_cpc(unsigned char *buf) {
    unsigned char ret = 0;
    for (int i = 0; i < 4; i++) {
	ret = ret << 1;
	ret |= ((buf[i] >> 6) & 1) | ((buf[i] >> 3) & 0x10);
    }
    return ret;
}

static void remove_extension(char *src, char *dst) {
    for (int i = 0; i < strlen(src); i++) {
	if (src[i] == '.') {
	    dst[i] = 0;
	    return;
	}
	else if (src[i] == '/') {
	    dst[i] = '_';
	}
	else {
	    dst[i] = src[i];
	}
    }
}

static char *file_name;
int main(int argc, char **argv) {
    if (argc != 3) {
	printf("USAGE: tga-dump [option] file.tga\n");
	printf("  -c   save bitmap cpc\n");
	printf("  -f   save font cpc\n");
	printf("  -b   save bitmap zx\n");
	printf("  -l   save level\n");
	return 0;
    }

    file_name = argv[2];
    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
	printf("ERROR: unable to open %s\n", file_name);
	return -ENOENT;
    }

    struct Header header;
    read(fd, &header, sizeof(header));

    if (header.image_type != 3 || header.depth != 8) {
	printf("ERROR: not a grayscale 8-bit TGA file\n");
	close(fd);
	return 0;
    }

    unsigned char buf[header.h * header.w];
    read(fd, buf, header.w * header.h);
    close(fd);

    switch (argv[1][1]) {
    case 'l':
	save_level(&header, buf);
	break;
    case 'b':
	save_bitmap(&header, buf);
	break;
    case 'c':
	save_bitmap_cpc(&header, buf);
	break;
    case 'f':
	save_font_cpc(&header, buf);
	break;
    }

    return 0;
}

static void hexdump(unsigned char *buf, int size) {
    for (int i = 0; i < size; i++) {
	fprintf(stderr, "%02x ", buf[i]);
	if ((i & 0xf) == 0xf) {
	    fprintf(stderr, "\n");
	}
    }
}

static void save_level(struct Header *header, unsigned char *buf) {
    if (header->w != WIDTH * 8) {
	printf("ERROR: bad width %d, expected %d\n", header->w, WIDTH * 8);
	exit(-1);
    }

    unsigned height = 0;
    unsigned char row[WIDTH];
    memset(row, 0, sizeof(row));

    unsigned size = 3;
    unsigned char out[(WIDTH * header->h + 1) * 3];
    memset(out, 0, size); /* zero for termination */

    for (int y = header->h - 1; y >= 0; y--) {
	unsigned char *ptr = buf + y * header->w;
	for (int i = 0; i < WIDTH; i++) {
	    unsigned char pixels = consume_pixels(ptr);
	    if (row[i] != pixels) {
		row[i] = pixels;
		out[size + 0] = i + START;
		out[size + 1] = height;
		out[size + 2] = pixels;
		size += 3;
		height = 0;
	    }
	    ptr += 8;
	}
	height++;
    }

    char name[256];
    remove_extension(file_name, name);
    printf("const byte %s[%d] = {\n", name, size);
    fprintf(stderr, "%s is %d bytes\n", name, size);
    for (int i = size - 3; i >= 0; i -= 3) {
	for (int j = 0; j < 3; j++) {
	    printf(" 0x%02x,", out[i + j]);
	}
	printf("\n");
    }
    printf("};\n");
}

static void save_bitmap(struct Header *header, unsigned char *buf) {
    char name[256];
    int size = header->w * header->h;
    remove_extension(file_name, name);
    printf("const byte %s[%d] = {\n", name, size / 8);
    for (int i = 0; i < size; i += 8) {
	printf(" 0x%02x,", consume_pixels(buf + i));
	if ((i % 64) == 56) printf("\n");
    }
    printf("};\n");
}

static void save_bitmap_cpc(struct Header *header, unsigned char *buf) {
    char name[256];
    int size = header->w * header->h;
    remove_extension(file_name, name);
    printf("const byte %s[%d] = {\n", name, size / 4);
    for (int i = 0; i < size; i += 4) {
	printf(" 0x%02x,", consume_pixels_cpc(buf + i));
	if ((i % 32) == 28) printf("\n");
    }
    printf("};\n");
}

static void save_font_cpc(struct Header *header, unsigned char *buf) {
    int i = 0;
    char name[256];
    int size = header->w * header->h;
    remove_extension(file_name, name);
    printf("const byte %s[%d] = {\n", name, size / 4);
    for (int row = 0; row < header->h; row += 8) {
	for (int x = 0; x < header->w; x += 8) {
	    for (int y = 0; y < 8; y++) {
		int offset = (row + y) * header->w + x;
		printf(" 0x%02x,", consume_pixels_cpc(buf + offset + 0));
		printf(" 0x%02x,", consume_pixels_cpc(buf + offset + 4));
		if ((i++ % 4) == 3) printf("\n");
	    }
	}
    }
    printf("};\n");
}

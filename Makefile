CFLAGS := -mz80 --nostdinc --nostdlib --no-std-crt0
CFLAGS += --code-loc 0x8000 --data-loc 0x9000

all:
	sdcc $(CFLAGS) main.c -o zlalox.ihx
	hex2bin zlalox.ihx > /dev/null
	./bin2tap.sh zlalox

run: all
	fuse -g 2x zlalox.tap

clean:
	rm -f zlalox*

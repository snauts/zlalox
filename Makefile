CFLAGS := -mz80 --nostdinc --nostdlib --no-std-crt0
CFLAGS += --code-loc 0x8000 --data-loc 0xf000

all:
	make level_data
	sdcc $(CFLAGS) main.c -o zlalox.ihx
	hex2bin zlalox.ihx > /dev/null
	./bin2tap.sh zlalox

level_data:
	rm level.h -f
	gcc tga-dump.c -o tga-dump
	$(foreach file, $(wildcard level/*), ./tga-dump $(file) >> level.h;)

run: all
	fuse --no-confirm-actions -g 2x zlalox.tap

clean:
	rm -f zlalox*

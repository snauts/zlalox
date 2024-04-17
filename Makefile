CFLAGS := -mz80 --nostdinc --nostdlib --no-std-crt0
CFLAGS += --code-loc 0x8000 --data-loc 0xf000

all:
	rm level.h -f
	gcc tga-dump.c -o tga-dump
	$(foreach F, $(wildcard level/*), ./tga-dump -l $(F) >> level.h;)
	@echo compile zlalox with sdcc
	@sdcc $(CFLAGS) main.c -o zlalox.ihx
	hex2bin zlalox.ihx > /dev/null
	./bin2tap.sh zlalox

run: all
	fuse --no-confirm-actions -g 2x zlalox.tap

clean:
	rm -f zlalox* level.h tga-dump

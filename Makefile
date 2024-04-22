CFLAGS += -mz80 --nostdinc --nostdlib --no-std-crt0
CFLAGS += --code-loc 0x8000 --data-loc 0xb000

ENTRY = grep _main zlalox.map | cut -d " " -f 6

all:
	make cpc

prg:
	rm level.h -f
	gcc tga-dump.c -o tga-dump
	@echo convert levels to headers
	@$(foreach F, $(wildcard level/*), ./tga-dump -l $(F) >> level.h;)
	./tga-dump -b title.tga >> level.h
	@echo compile zlalox with sdcc
	@sdcc $(CFLAGS) main.c -o zlalox.ihx
	hex2bin zlalox.ihx > /dev/null

zxs: prg
	bin2tap -b -r $(shell printf "%d" 0x$$($(ENTRY))) zlalox.bin
	fuse --no-confirm-actions -g 2x zlalox.tap

cpc: prg
	iDSK -n zlalox.dsk
	iDSK zlalox.dsk -f -t 1 -c 8000 -e $(shell $(ENTRY)) -i zlalox.bin
	mame cpc6128 \
		-window \
		-skip_gameinfo \
		-flop1 zlalox.dsk \
		-autoboot_delay 1 \
		-ab "RUN \"ZLALOX.BIN\"\n"

clean:
	rm -f zlalox* level.h tga-dump

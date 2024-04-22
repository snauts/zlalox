CFLAGS += -mz80 --nostdinc --nostdlib --no-std-crt0
CFLAGS += --code-loc 0x8000 --data-loc 0xb000

ENTRY = printf "%d" 0x$$(grep _main zlalox.map | cut -d " " -f 6)

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

zxs:
	CFLAGS=-DZXS make prg
	bin2tap -b -r $(shell $(ENTRY)) zlalox.bin
	fuse --no-confirm-actions -g 2x zlalox.tap

cpc:
	CFLAGS=-DCPC make prg
	iDSK -n zlalox.dsk
	unix2dos -n cpc.bas zlalox.bas
	iDSK zlalox.dsk -f -t 0 -i zlalox.bas
	iDSK zlalox.dsk -f -t 1 -i zlalox.bin
	mame cpc6128 \
		-window \
		-skip_gameinfo \
		-flop1 zlalox.dsk \
		-autoboot_delay 1 \
		-ab "RUN \"ZLALOX.BAS\"\n"

clean:
	rm -f zlalox* level.h tga-dump

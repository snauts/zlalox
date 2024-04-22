CFLAGS += -mz80 --nostdinc --nostdlib --no-std-crt0
CFLAGS += --code-loc $(CODE) --data-loc $(DATA)

ENTRY = grep _main zlalox.map | cut -d " " -f 6

all:
	make cpc

dmp:
	gcc $(TYPE) tga-dump.c -o tga-dump

prg:
	@echo convert levels to headers
	@$(foreach F, $(wildcard level/*), ./tga-dump -l $(F) >> level.h;)
	@echo compile zlalox with sdcc
	sdcc $(CFLAGS) $(TYPE) main.c -o zlalox.ihx
	hex2bin zlalox.ihx > /dev/null

zxs_bin: dmp
	./tga-dump -b title.tga > level.h
	CODE=0x8000 DATA=0xf000	TYPE=-DZXS make prg

cpc_bin: dmp
	./tga-dump -c title_cpc.tga > level.h
	CODE=0x4000 DATA=0x7000	TYPE=-DCPC make prg

zxs: zxs_bin
	bin2tap -b -r $(shell printf "%d" 0x$$($(ENTRY))) zlalox.bin
	fuse --no-confirm-actions -g 2x zlalox.tap

cpc: cpc_bin
	iDSK -n zlalox.dsk
	iDSK zlalox.dsk -f -t 1 -c 4000 -e $(shell $(ENTRY)) -i zlalox.bin
	mame cpc6128 \
		-window \
		-skip_gameinfo \
		-flop1 zlalox.dsk \
		-autoboot_delay 1 \
		-ab "RUN \"ZLALOX.BIN\"\n"

clean:
	rm -f zlalox* level.h tga-dump

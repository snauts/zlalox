CFLAGS += -mz80 --nostdinc --nostdlib --no-std-crt0
CFLAGS += --code-loc $(CODE) --data-loc $(DATA)

ENTRY = grep _main zlalox.map | cut -d " " -f 6

all:
	@echo "make zxs" - build .tap for ZX Spectrum
	@echo "make cpc" - build .dsk for Amstrad CPC
	@echo "make cdt" - build .cdt for Amstrad CPC
	@echo "make cap" - build and run caprice
	@echo "make fuse" - build and run fuse
	@echo "make mame" - build and run mame

dmp:
	gcc $(TYPE) tga-dump.c -o tga-dump

dmp_cpc:
	TYPE=-DBPP=2 make dmp

prg:
	@echo convert levels to headers
	@$(foreach F, $(wildcard level/*), ./tga-dump -l $(F) >> level.h;)
	@echo compile zlalox with sdcc
	@sdcc $(CFLAGS) $(TYPE) main.c -o zlalox.ihx
	hex2bin zlalox.ihx > /dev/null

zxs_bin: dmp
	./tga-dump -b title.tga > level.h
	CODE=0x8000 DATA=0xf000	TYPE=-DZXS make prg

cpc_bin: dmp_cpc
	./tga-dump -c title_cpc.tga > level.h
	./tga-dump -c skii_mask.tga >> level.h
	./tga-dump -f font_cpc.tga >> level.h
	./tga-dump -c crown.tga >> level.h
	CODE=0x4000 DATA=0x9000	TYPE=-DCPC make prg

zxs: zxs_bin
	bin2tap -b -r $(shell printf "%d" 0x$$($(ENTRY))) zlalox.bin

fuse: zxs
	fuse --no-confirm-actions -g 2x zlalox.tap

cpc: cpc_bin
	iDSK -n zlalox.dsk
	iDSK zlalox.dsk -f -t 1 -c 4000 -e $(shell $(ENTRY)) -i zlalox.bin

cdt: cpc_bin
	2cdt -n -P -t 0 -F 2 -L 0x4000 -X 0x$(shell $(ENTRY)) \
		-r zlalox zlalox.bin zlalox.cdt
#	cap32 zlalox.cdt -a "|TAPE" -a "RUN\"\""

mame: cpc
	mame cpc6128 \
		-window \
		-skip_gameinfo \
		-flop1 zlalox.dsk \
		-autoboot_delay 1 \
		-ab "RUN \"ZLALOX.BIN\"\n"

cap: cpc
	cap32 zlalox.dsk -a "RUN \"ZLALOX.BIN\""

clean:
	rm -f zlalox* level.h tga-dump

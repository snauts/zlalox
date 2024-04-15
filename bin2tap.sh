#!/bin/bash

ENTRY=$(grep _main $1.map | cut -d " " -f 6)
bin2tap -b -r $(printf "%d" 0x$ENTRY) $1.bin

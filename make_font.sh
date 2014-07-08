#!/bin/bash

gfx=$1
headerfile='font.h'
pbm=/tmp/$RANDOM.pbm

convert -compress none $gfx -rotate 90 $pbm

(\
	echo '#include <avr/pgmspace.h>'
	echo
	echo 'static const uint8_t font[] PROGMEM = {'
	( \
		echo 'ibase=2;' \
		&& cat $pbm \
			| sed '1d;2d' \
			| tr -d '\n ' \
			| sed -r 's/([01]{8})/\1\n/g' \
	) \
		| bc \
		| awk '(NR-1)%6!=5 {printf("0x%.2X, ", $0)} (NR-1)%6==5 {printf("\n")}' \
		| sed -r 's/^/\t/g' \
		| sed -r 's/ $//g' \
		| sed -r '$s/,$//g'
	echo '};'
) > font.h

rm -fr $pbm

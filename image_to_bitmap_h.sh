#!/bin/bash

gfx=$1
font_mode=$2
headerfile='font.h'
pbm=/tmp/$RANDOM.pbm

if [ "$font_mode" == font ]; then
	convert -compress none $gfx -rotate 90 $pbm
else
	convert -compress none $gfx $pbm
fi


(\
	if [ "$font_mode" == font ]; then
		echo '#include <avr/pgmspace.h>'
		echo
		echo 'static const uint8_t font[] PROGMEM = {'
	fi
	( \
		echo 'ibase=2;' \
		&& cat $pbm \
			| sed '1d;2d' \
			| tr -d '\n ' \
			| sed -r 's/([01]{8})/\1\n/g' \
	) \
		| bc > $pbm.h
	( \
		if [ "$font_mode" == font ]; then
			awk '(NR-1)%6!=5 {printf("0x%.2X, ", $0)} (NR-1)%6==5 {printf("\n")}' < $pbm.h
		else
			awk '// {printf("0x%.2X, ", $0)} (NR-1)%8==7 {printf("\n")} END {printf("\n")}' < $pbm.h
		fi
	) \
		| sed -r 's/^/\t/g' \
		| sed -r 's/ $//g' \
		| sed -r '$s/,$//g'
	if [ "$font_mode" == font ]; then
		echo '};'
	fi
)

rm -fr $pbm
rm -fr $pbm.h

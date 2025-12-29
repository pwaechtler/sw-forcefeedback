#!/bin/sh
NMTOOL=$HOME/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avr-nm 

if [ "${1}not" = "not" ]
then
	echo ${0} has to be called with the .elf file as argument, for example:
	echo $0 /var/folders/lw/3k6209j15c7052nz45w8hf2m0000gq/T/arduino_build_791659/LUFA-ffb.ino.elf
else
	${NMTOOL} --size-sort --radix=d --demangle ${1} 
fi

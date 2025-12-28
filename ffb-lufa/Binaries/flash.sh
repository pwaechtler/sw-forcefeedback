#!/bin/sh
# this is for mac OS
# on Linux it would be PORT=$(ls /dev/ttyUSB*)

echo "reset the board wait 1 second and press RETURN"
read
PORT=$(ls /dev/cu.usb*)
#avrdude -c avr109 -p m32u4 -U flash:w:gameport-adapter.ino.micro.hex -P /dev/cu.usbmodem14101
avrdude -c avr109 -p m32u4 -U flash:w:LUFA-ffb.ino.micro.hex   -P $PORT


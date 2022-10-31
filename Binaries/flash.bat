echo check the COM port number

avrdude.exe -u -c avr109 -p m32u4 -P \\.\com8  -U flash:w:LUFA-ffb.ino.micro.hex

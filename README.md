# Dev info

running makefile will create an unoFirmware.hex file that can be flashed (uses avr-gcc for compiling) 
premade run.sh script to flash the hex file to arduino uno. Paths and ports need to be modified, current settings for arch linux.


Mega2560 avrdude command: sudo avrdude -c stk500v2 -v -V  -P /dev/ttyACM0 -b 115200 -p m2560 -D -U flash:w:megaFirmware.hex:i  
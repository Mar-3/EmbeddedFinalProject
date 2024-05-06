#!/bin/bash

#change paths to what works, needs sudo if some settings arent configured in arch
"/home/marko/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/bin/avrdude" "-C/home/marko/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/etc/avrdude.conf" -v -V -patmega328p -carduino "-P/dev/ttyACM1" -b115200 -D "-Uflash:w:unoFirmware.hex:i"
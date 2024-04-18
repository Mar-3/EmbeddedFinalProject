

main: main.o keypad.o
	avr-gcc main.o -o main

main.o: main.c
	avr-gcc main.c -c -o main.o

keypad.o: keypad.c io.o
	avr-gcc -v keypad.c -c -o keypad.o

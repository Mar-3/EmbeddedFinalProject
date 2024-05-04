# Used https://wiki.archlinux.org/title/AVR
# TODO should probably get the external libraries from
# a folder, this is messy right now

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -f

MCU = atmega328p
F_CPU = 8000000

LFUSE = 0x9f
HFUSE = 0xd1

TARGET = unoFirmware
SRC = main.c delay.c
OBJ = $(SRC:.c=.o)
LST = $(SRC:.c=.lst)

FORMAT = ihex

OPTLEVEL = s

CDEFS = 

CFLAGS = -DF_CPU=$(F_CPU)UL
CFLAGS += $(CDEFS)
CFLAGS += -O$(OPTLEVEL)
CFLAGS += -mmcu=$(MCU)
CFLAGS += -std=gnu99
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -Wa,-adhlns=$(<:.c=.lst)

LDFLAGS = -Wl,--gc-sections
LDFLAGS += -Wl,--print-gc-sections

AVRDUDE_MCU = atmega328p
AVRDUDE_PROGRAMMER = arduino
AVRDUDE_SPEED = -B9600

AVRDUDE_FLAGS = -p $(AVRDUDE_MCU)
AVRDUDE_FLAGS += -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += $(AVRDUDE_SPEED)

MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_FLASH = Preparing HEX file:

all: gccversion $(TARGET).elf $(TARGET).hex size

.SECONDARY: $(TARGET).elf
.PRECIOUS: $(OBJ)

%.hex: %.elf
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -j .text -j .data $< $@

%.elf: $(OBJ)
	@echo $(MSG_LINKING) $@
	$(CC) -mmcu=$(MCU) $(LDFLAGS) $^ --output $(@F)

%.o : %.c
	@echo $(MSG_COMPILING) $<
	$(CC) $(CFLAGS) -c $< -o $(@F)

gccversion:
	@$(CC) --version

size: $(TARGET).elf
	@echo
	$(SIZE) -C --mcu=$(AVRDUDE_MCU) $(TARGET).elf

analyze: $(TARGET).elf
	$(NM) -S --size-sort -t decimal $(TARGET).elf

isp: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$(TARGET).hex

fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m

release: fuses isp

clean:
	$(REMOVE) $(TARGET).hex $(TARGET).elf $(OBJ) $(LST) *~
# AVR-GCC Makefile
#---Prefs---
PROJECT=rbds_transmitter
MMCU=atmega328p
F_CPU=16000000 # 16 MHz

LFUSEBITS=0xE2
HFUSEBITS=0xDE
EFUSEBITS=0xFD
#-----------



SOURCES=main.c lcd.c spi.c uart.c
CC=avr-gcc
OBJCOPY=avr-objcopy

CFLAGS=-g -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wstrict-prototypes -DF_CPU=$(F_CPU) -Wa,-adhlns=$(<:.c=.lst) -I./ -mmcu=$(MMCU) -Wall
AVRDUDEFLAGS=-p $(MMCU)

ALL: $(PROJECT).hex filesize

$(PROJECT).hex: $(PROJECT).elf
	avr-objcopy -j .text -O ihex $(PROJECT).elf $(PROJECT).hex

$(PROJECT).elf: $(SOURCES)
	$(CC) $(CFLAGS) -o $(PROJECT).elf $(SOURCES)

program-isp: $(PROJECT).hex
	avrdude $(AVRDUDEFLAGS) -c usbasp -F -e -U flash:w:$(PROJECT).hex

program: $(PROJECT).hex
	avrdude $(AVRDUDEFLAGS) -c stk500v1 -P /dev/ttyUSB0 -b 19200 -D -U flash:w:$(PROJECT).hex:i

burn-fuse:
	avrdude $(AVRDUDEFLAGS) -c usbasp -F -u -U lfuse:w:$(LFUSEBITS):m -U hfuse:w:$(HFUSEBITS):m -U efuse:w:$(EFUSEBITS):m

# Display size of file.
filesize:
	avr-size -C --mcu=$(MMCU) $(PROJECT).elf 

clean:
	rm -f $(PROJECT).elf $(PROJECT).hex $(PROJECT).lst

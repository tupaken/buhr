TARGET=main
MCU=atmega48a
SOURCES=main.c
MCU_dude=m48

# Программатор
PROGRAMMER=usbasp
PORT=-P usb
BAUD=-B115200

# Компилятор
CC=avr-gcc  # Добавлено здесь!
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-c -Os -Wall -Wextra -DF_CPU=1000000UL -mmcu=$(MCU)
LDFLAGS=-mmcu=$(MCU)

all: hex eeprom

hex: $(TARGET).hex

eeprom: $(TARGET)_eeprom.hex

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -j .text $(TARGET).elf $(TARGET).hex

$(TARGET)_eeprom.hex: $(TARGET).elf
	avr-objcopy -O ihex -j .eeprom --change-section-lma .eeprom=1 $(TARGET).elf $(TARGET)_eeprom.hex

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET).elf

.c.o:
	$(CC) $(CFLAGS) $< -o $@

size:
	avr-size --mcu=$(MCU) -C $(TARGET).elf

program:
	avrdude -p$(MCU_dude) $(PORT) $(BAUD) -c$(PROGRAMMER) -Uflash:w:$(TARGET).hex:a

fuse:
	avrdude -p $(MCU_dude) -c $(PROGRAMMER) $(PORT) -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m

clean_tmp:
	rm -rf *.o
	rm -rf *.elf

clean:
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.hex

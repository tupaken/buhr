## Как запускать

- залисчу команды по порядку, как я это на ардуино делаю
- скорее всего с мейкфайлом тоже сработает, эт как запасной варик

## Step by step guide:


1. Запустить avr-gcc скрипт

Далее вставить команды по порядку:

```
avr-gcc -mmcu=atmega48 -Os -DF_CPU=1000000UL -c main.c -o main.o
```

```
avr-gcc -mmcu=atmega48 main.o -o main.elf
```

```
avr-objcopy -O ihex -R .eeprom main.elf main.hex
```

```
avrdude -p m48 -c usbasp -U flash:w:main.hex:i -B 10
```

В конце обязательно должна появиться небольшая дорожка загрузки
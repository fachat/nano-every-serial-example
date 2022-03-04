
SERIAL=/dev/ttyACM0

CFLAGS=-g -Os -funsigned-char -fshort-enums 
CFLAGS+=-Wall -pedantic -Wextra -DFIRMWARE
CFLAGS+=-std=gnu11 -O2 $(MCU_CC) $(INCLUDE) $(SDEFS)

LDFLAGS=-Wl,-u,vfprintf -lprintf_flt

MCU_CC=-mmcu=atmega4809

all: serial.hex blink.hex serial_with_int.hex

%.hex: %.elf
	avr-objcopy -j .data -j .text -O ihex $< $@

%.elf: %.o
	avr-gcc $(MCU_CC) -o $@ $(LDFLAGS) $<

%.o: %.c
	avr-gcc $(CFLAGS) -c $< -o $@

load_serial_with_int: serial_with_int.hex reset
	avrdude -C ./avrdude.conf -c jtag2updi -P $(SERIAL) -p atmega4809 -u \
		-U flash:w:serial_with_int.hex:i

load_serial: serial.hex reset
	avrdude -C ./avrdude.conf -c jtag2updi -P $(SERIAL) -p atmega4809 -u \
		-U flash:w:serial.hex:i

load_blink: blink.hex reset
	avrdude -C ./avrdude.conf -c jtag2updi -P $(SERIAL) -p atmega4809 -u \
		-U flash:w:blink.hex:i

readfuses: reset
	avrdude -C ./avrdude.conf -c jtag2updi -P $(SERIAL) -p atmega4809 -u \
		-U fuse2:r:fuse2_OSCCFG.txt:h \
		-U fuse5:r:fuse5_SYSCFG0.txt:h \
		-U fuse8:r:fuse8_BOOTEND.txt:h

reset:
	# force reset
	sudo stty -F $(SERIAL) hup ispeed 1200
	sudo stty -F $(SERIAL) hup ispeed 1200	

clean:
	rm -f *.o *.elf *.hex


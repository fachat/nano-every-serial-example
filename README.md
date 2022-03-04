
This repo shows how to setup the Arduino Nano Every with Linux, 
and also how to use the serial port (the one of the four that is connected 
to the USB interface)

blink.c: a small program to blink the TX LED on the nano every.

serial.c: setup the serial interface and use it without interrupts

serial_with_int.c: setup the serial interface and use it with interrupts

Makefile: documents the commands how to build and load the program into flash via the USB/serial interface


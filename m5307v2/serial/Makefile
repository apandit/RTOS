
# Makefile
# David Grant, 2004
# Irene Huang, 2010/05/03

CC=m68k-elf-gcc
CXX=m68k-elf-g++
#CFLAGS=-Wall -m5307 -pipe -nostdlib
CFLAGS=-Wall -m5307 -pipe -nostdlib -D_DEBUG_ 
#CFLAGS=-Wall -m5307 -pipe -nostdlib -D_CFSERVER_ -D_DEBUG_
LD=m68k-elf-gcc
AS=m68k-elf-as
AR=m68k-elf-ar
ARFLAGS=
OBJCPY=m68k-elf-objcopy
OBJDMP=m68k-elf-objdump
ASM=../shared/start.s serial_entry.s
LDFLAGS=-T../shared/rtx.ld -Wl,-Map=serial.map
#LDFLAGS=-T../shared/mcf5307.ld -Wl,-Map=serial.map

SERIAL_OBJS=dbug.o serial.o

# Note, GCC builds things in order, it's important to put yhe
#  ASM first, so that it is located at the beginning of our program.
serial.s19: serial_entry.s $(SERIAL_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o serial.bin $(ASM) $(SERIAL_OBJS) 
	$(OBJCPY) --output-format=srec serial.bin serial.s19
	$(OBJDMP) -xdC serial.bin > serial.lst
	chmod u+x serial.s19

serial.o: serial.c
	$(CC) $(CFLAGS) -c serial.c

dbug.o: dbug.c dbug.h
	$(CC) $(CFLAGS) -c dbug.c

clean:
	rm -f *.s19 *.o *.bin *.lst *.map

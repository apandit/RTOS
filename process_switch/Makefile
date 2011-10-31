
# Makefile
# David Grant, 2004
# Irene Huang, 2010/05/03
# Modified 2010/05/23 by apandit igrabovi ltng jlfeng

CC=m68k-elf-gcc
CXX=m68k-elf-g++
CFLAGS= -Wall -m5307 -pipe -nostdlib
#CFLAGS= -Wall -m5307 -pipe -nostdlib -D_DEBUG_
#CFLAGS= -Wall -m5307 -pipe -nostdlib -D_DEBUG_ -D_CFSERVER_
LD=m68k-elf-gcc
AS=m68k-elf-as
AR=m68k-elf-ar
ARFLAGS=
OBJCPY=m68k-elf-objcopy
OBJDMP=m68k-elf-objdump
ASM=../shared/start.s trap_entry.s
LDFLAGS = -T../shared/rtx.ld -Wl,-Map=proc_switch.map

PSWTCH_OBJS=dbug.o proc_switch.o pstruct.o

# Note, GCC builds things in order, it's important to put yhe
#  ASM first, so that it is located at the beginning of our program.
proc_switch.s19: $(PSWTCH_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o proc_switch.bin $(ASM) $(PSWTCH_OBJS) 
	$(OBJCPY) --output-format=srec proc_switch.bin proc_switch.s19
	$(OBJDMP) -xdC proc_switch.bin > proc_switch.lst

proc_switch.o: proc_switch.c
	$(CC) $(CFLAGS) -c proc_switch.c

pstruct.o: pstruct.c pstruct.h
	$(CC) $(CFLAGS) -c pstruct.c

dbug.o: dbug.c dbug.h
	$(CC) $(CFLAGS) -c dbug.c

clean:
	rm -f *.s19 *.o *.bin *.map *.lst

# Makefile Spring 2010 
# by
# David Grant
# Thomas Reidemeister
# Irene Huang
# apandit igrabovi ltng jlfeng

CC=m68k-elf-gcc
CXX=m68k-elf-g++
#CFLAGS= -Wall -m5307 -pipe -nostdlib
CFLAGS= -Wall -m5307 -pipe -nostdlib -D_DEBUG_ -D_DEBUG_HOTKEYS
#CFLAGS= -Wall -m5307 -pipe -nostdlib -D_DEBUG_ -D_CFSERVER_ -D_DEBUG_HOTKEYS
LD=m68k-elf-gcc
AS=m68k-elf-as
AR=m68k-elf-ar
ARFLAGS=
OBJCPY=m68k-elf-objcopy
OBJDUMP=m68k-elf-objdump
ASM=./start.s 
RTXASM=./start.s trap_entry.s uart.s timer.s
LDFLAGS_RTX = -Trtx.ld -Wl,-Map=rtx.map
LDFLAGS_RTX_LOADER = -Trtx_loader.ld -Wl,-Map=rtx_loader.map
LDFLAGS_RTX_TEST = -Trtx_test.ld -Wl,-Map=rtx_test.map

RTX_OBJS = rtx.o dbug.o main_rtx.o pcb.o mem.o processmanager.o sched.o queues.o kernel.o uart.o timer.o wallclock.o strings.o priority.o testprocs.o
RTX_LOADER_OBJS = dbug.o rtx_loader.o
RTX_TEST_DUMMY_OBJS = dbug.o rtx_test_dummy.o

all: mdummy.s19 


# Note, GCC builds things in order, it's important to put yhe
#  ASM first, so that it is located at the beginning of our program.

rtx.s19: $(RTX_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS_RTX) -o rtx.bin $(RTXASM) $(RTX_OBJS) 
	$(OBJCPY) --output-format=srec rtx.bin rtx.s19
	$(OBJDUMP) -xdC rtx.bin > rtx.lst

rtx_loader.s19: $(RTX_LOADER_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_LOADER) -o rtx_loader.bin $(ASM) $(RTX_LOADER_OBJS) 
	$(OBJCPY) --output-format=srec rtx_loader.bin rtx_loader.s19
	$(OBJDUMP) -xdC rtx_loader.bin > rtx_loader.lst

rtx_test_dummy.s19: $(RTX_TEST_DUMMY_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o rtx_test_dummy.bin $(ASM) $(RTX_TEST_DUMMY_OBJS) 
	$(OBJCPY) --output-format=srec rtx_test_dummy.bin rtx_test_dummy.s19
	$(OBJDUMP) -xdC rtx_test_dummy.bin > rtx_test_dummy.lst

m2.s19: rtx_test_dummy.s19 rtx.s19
	python merge.py m2.s19 rtx.s19 rtx_test_dummy.s19 
	chmod u+x m2.s19

mdummy.s19: m2.s19 rtx_loader.s19
	python merge.py mdummy.s19 m2.s19 rtx_loader.s19 
	chmod u+x mdummy.s19

.c.o:
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -f *.bin *.s19 *.o *.map *.lst

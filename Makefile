CFLAGS=-fno-stack-protector -std=gnu99 -Os -fno-pic -nostdlib -m32 -march=i486 -ffreestanding -fno-builtin -fno-asynchronous-unwind-tables -fno-exceptions
CC=gcc
SRC = $(wildcard *.c)
OBJECTS = $(SRC:%.c=%.o)


rom: $(OBJECTS)
	ld -m elf_i386 $(OBJECTS) -r -o c_code.o
	ld -T rom.ld c_code.o -o c_code.bin 
	nasm -f bin reset.s -o bios.bin

$(OBJECTS): %.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@


clean:
	-rm *.o *.bin

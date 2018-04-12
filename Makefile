CFLAGS=-fno-stack-protector -std=gnu99 -Os -fno-pic -nostdlib -m32 -march=i486 -ffreestanding -fno-builtin -fno-asynchronous-unwind-tables -fno-exceptions
CC=gcc
C_SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.asm)
ASM_BLOB_SRC = $(wildcard *.S)
C_OBJECTS = $(C_SRC:%.c=%.o)
ASM_OBJECTS = $(ASM_SRC:%.asm=%.o)
ASM_BLOBS = $(ASM_BLOB_SRC:%.S=%.bin)

bios.bin: $(C_OBJECTS) $(ASM_OBJECTS)
	ld -T rom.ld $^ -o $@

$(O_OBJECTS): %.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

$(ASM_OBJECTS): %.o: %.asm $(ASM_BLOBS) 
	nasm -f elf $< -o $@

$(ASM_BLOBS): %.bin: %.S
	nasm -f bin $^ -o $@


clean:
	-rm *.o *.bin

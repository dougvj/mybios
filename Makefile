

CFLAGS=-fno-stack-protector \
	   -std=gnu99 \
	   -Os \
	   -fno-pic \
	   -nostdlib \
	   -m32 \
	   -march=i486\
	   -ffreestanding \
	   -fno-builtin \
	   -fno-asynchronous-unwind-tables \
	   -fno-exceptions \
	   -I./

CC=gcc
CHIPSET=null
#CHIPSET=sis8c460
C_SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.asm)
ASM_BLOB_SRC = $(wildcard *.S)
C_OBJECTS = $(C_SRC:%.c=%.o)
ASM_OBJECTS = $(ASM_SRC:%.asm=%.o)
ASM_BLOBS = $(ASM_BLOB_SRC:%.S=%.bin)

all: bios.bin

debug: bios.bin bios.sym 
	bochs -q

rom.ld: rom_template.ld 
	cpp -P rom_template.ld -o rom.ld

bios.sym: bios.elf
	objdump -t bios.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > bios.sym

bios.bin: bios.elf
	objcopy -O binary bios.elf bios.bin

bios.elf: rom.ld $(C_OBJECTS) $(ASM_OBJECTS) chipset.o
	ld -T $^ -o bios.elf

$(C_OBJECTS): %.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

chipset.o: chipsets/$(CHIPSET).c 
	$(CC) -c $^ $(CFLAGS) -o $@

$(ASM_OBJECTS): %.o: %.asm $(ASM_BLOBS) 
	nasm -f elf $< -o $@

$(ASM_BLOBS): %.bin: %.S
	nasm -f bin $^ -o $@

clean:
	-rm *.o *.bin *.elf *.sym rom.ld

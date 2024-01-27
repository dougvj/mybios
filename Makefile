

CFLAGS=-fno-stack-protector \
			 -std=gnu2x \
			 -Os \
			 -fno-pic \
			 -nostdlib \
			 -m32 \
			 -march=i386\
			 -mgeneral-regs-only \
			 -ffreestanding \
			 -fno-builtin \
			 -fno-asynchronous-unwind-tables \
			 -fno-exceptions \
			 -Wno-unused-parameter \
			 -Wno-unused-function \
			 -Wno-array-bounds \
			 -funsigned-char \
			 -Wall \
			 -Wextra \
			 -I./

CFLAGS += -Werror
CC=gcc
CHIPSET=440fx
#CHIPSET=null
#CHIPSET=sis8c460
#CHIPSET=um82c480
C_SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.asm)
ASM_BLOB_SRC = $(wildcard *.S)
C_OBJECTS = $(C_SRC:%.c=%_c.o)
ASM_OBJECTS = $(ASM_SRC:%.asm=%_asm.o)
ASM_BLOBS = $(ASM_BLOB_SRC:%.S=%_S.bin)

all: bios.bin

ROM_SIZE=64k

DEFINES=
DEFINES += -DBOCHS
#DEFINES += -DDEBUG
#DEFINES += -DENABLE_SERIAL
#DEFINES += -DENABLE_EARLY_SERIA
#DEFINES += -DENABLE_EARLY_VIDEO
DEFINES += -DENABLE_PCI
CFLAGS += $(DEFINES)
ASMFLAGS += $(DEFINES)

DEFINES += -DROM_SIZE=${ROM_SIZE}

bochs: bios.bin bios.sym
	-rm bochs.img.lock
	bochs -q

qemu: bios.bin bios.sym
	./qemu.sh

padded: bios.bin
	echo -n -e '\xEF\xBE\xAD\xDE' > __signature
	dd if=/dev/zero of=__padding count=65532 bs=1
	cat __signature __padding > __bank
	cat __bank bios.bin > padded_bios_128k.bin
	cat __bank __bank __bank bios.bin > padded_bios_256k.bin
	rm __padding __signature __bank
	chmod +x  padded_bios_*.bin

rom.ld: rom_template.ld
	cpp -DSTAY_IN_LOW_MEMORY -DROM_SIZE=${ROM_SIZE} -P rom_template.ld -o rom.ld

bios.sym: bios.elf
	objdump -t bios.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > bios.sym

bios.bin: bios.elf
	objcopy -O binary bios.elf bios.bin

bios.elf: rom.ld $(C_OBJECTS) $(ASM_OBJECTS) backend_chipset.o
	ld -T $^ -o bios.elf

eprom-emu: bios.bin
	eprom-emu-ng ./bios.bin /dev/ttyUSB0


$(C_OBJECTS): %_c.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

backend_chipset.o: chipsets/$(CHIPSET).c
	$(CC) -c $^ $(CFLAGS) -o $@

$(ASM_OBJECTS): %_asm.o: %.asm $(ASM_BLOBS)
	nasm -f elf $< $(ASMFLAGS) -o $@

$(ASM_BLOBS): %_S.bin: %.S
	nasm -f bin $^ $(ASMFLAGS) -o $@

clean:
	-rm *.o *.bin *.elf *.sym rom.ld

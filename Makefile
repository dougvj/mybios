

CFLAGS=-fno-stack-protector \
			 -std=gnu2x \
			 -Os \
			 -fno-pic \
			 -fno-builtin \
			 -march=i386\
			 -mgeneral-regs-only \
			 -Wno-unused-parameter \
			 -Wno-unused-function \
			 -Wno-array-bounds \
			 -Wno-main \
			 -funsigned-char \
			 -Wall \
			 -Wextra \
			 -I./

CFLAGS += -Werror
# Check for i386-elf-gcc
ifeq (, $(shell which i386-elf-gcc))
CC=x86_64-pc-linux-gnu-gcc
LD=x86_64-pc-linux-gnu-ld
OBJDUMP=x86_64-pc-linux-gnu-objdump
OBJCOPY=x86_64-pc-linux-gnu-objcopy
CFLAGS += -ffreestanding -nostdlib -m32
$(warning i386-elf-gcc not found, using x86_64-pc-linux-gnu-gcc)
else
CC=i386-elf-gcc
LD=i386-elf-ld
OBJDUMP=i386-elf-objdump
OBJCOPY=i386-elf-objcopy
endif
# This is the C preprocessor, not the C++
CPP=cpp
#CHIPSET=440fx
#CHIPSET=null
#CHIPSET=sis8c460
#CHIPSET=um82c480
# If chipset isn't set throw an error
ifeq ($(CHIPSET),)
# Get a list from the chipsets directory
CHIPSETs=$(shell ls chipsets | grep \\.c | sed 's/\.c//')
$(error CHIPSET is not set, please set it to one of: $(CHIPSETs))
endif
C_SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.asm)
ASM_BLOB_SRC = $(wildcard *.S)
C_OBJECTS = $(C_SRC:%.c=%_c.o)
ASM_OBJECTS = $(ASM_SRC:%.asm=%_asm.o)
ASM_BLOBS = $(ASM_BLOB_SRC:%.S=%_S.bin)

all: bios.bin

ROM_SIZE=64k

DEFINES=
#DEFINES += -DBOCHS
#DEFINES += -DDEBUG
#DEFINES += -DENABLE_SERIAL
#DEFINES += -DENABLE_EARLY_SERIAL
#DEFINES += -DENABLE_EARLY_VIDEO
DEFINES += -DENABLE_PCI
CFLAGS += $(DEFINES)
ASMFLAGS += $(DEFINES)

DEFINES += -DROM_SIZE=${ROM_SIZE}

bochs: bios.bin bios.sym
	-rm *.img.lock
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
	$(CPP) -DSTAY_IN_LOW_MEMORY -DROM_SIZE=${ROM_SIZE} -P rom_template.ld -o rom.ld

bios.sym: bios.elf
	$(OBJDUMP) -t bios.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > bios.sym

bios.bin: bios.elf
	$(OBJCOPY) -O binary bios.elf bios.bin

bios.elf: rom.ld $(C_OBJECTS) $(ASM_OBJECTS) backend_chipset.o
	$(LD) -T $^ -o bios.elf

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

#include "io.h"
#include "output.h"
#include "post.h"
#include "postcode.h"

static void writeReg(char reg, char u8) {
    outb(0x22, reg);
    outb(0x23, u8);
}

static unsigned char readReg(char reg) {
    outb(0x22, reg);
    return inb(0x23);
}

static void configDram(unsigned char cfg) {
    writeReg(0x50, cfg);
}

static unsigned char readDramConfig() {
    return readReg(0x50);
}

static int detectMirroring(unsigned int size) {
    for (int i = 0x100000; i < size; i+= 0x80000) {
        int* probe = (int*) i;
        *probe = i;
    }
    for (int i = 0x100000; i < size; i+= 0x80000) {
        int* probe = (int*) i;
        if (*probe != i) {
            return 1;
        }
    }
    return 0;
}

extern void main();
static void detectDramConfig() {
    if (readDramConfig() == 0) {
        printf("DRAM has not been initialized\n");
        unsigned char best_cfg = 0;
        unsigned int most_ram = 0;
        for (unsigned char i = 0; i < 32; i++) {
            postCode(i);
            configDram(i);
            register unsigned int ram = probeRam(0x100000, 32768, 0);
            register int mirroring = detectMirroring(ram);
            //Switch back to reset config so variables are the same
            configDram(0x0);
            if (!mirroring) {
                if (ram > most_ram) {
                    best_cfg = i;
                    most_ram = ram;
                }
            }
        }
        //Preserve video interrupt handler that was maybe setup by the video ROM
        register int vga_ivt = *(volatile int*)(0x40);
        configDram(best_cfg);
        if (vga_ivt != *(volatile int*)(0x40)) {
            *(int*)(0x40) = vga_ivt;
            cls();
            printf("Resetting IVT 0x%x\n", vga_ivt);
            for (int i = 0; i < 1000000; i++)
                asm("");
        }
        //Reset the stack
        asm("mov $0x2000, %esp\n"
            "mov $0x2000, %ebp");
        asm("jmp main");
    } else {
        printf("DRAM Initialized\n");
    }
}

#define DRAM_FASTEST 0x3
#define DRAM_FASTER  0x2
#define DRAM_SLOWER  0x1
#define DRAM_SLOWEST 0x0
#define CAS_1T  0x1
#define CAS_2T  0x0
static void setRamSpeed(int speed, int cas) {
    printf("Setting DRAM Speed...");
    char c = readDramConfig();
    c &= 0x1F;
    c |= speed << 6;
    c |= cas << 5;
    configDram(c);
    printf("OK\n");
}

#define CACHE_32KB 0x00
#define CACHE_64KB 0x01
#define CACHE_128KB 0x02
#define CACHE_256KB 0x03
static void setCPUCache(int en, int size, int write_back_en, int interleave, int write_cycle_2t, int burst_read) {
    printf("Enabling CPU Cache...");
    char c = 0;
    c |= burst_read;
    c |= (write_cycle_2t << 1);
    c |= (en << 2);
    c |= (interleave << 3);
    c |= (size << 4);
    c |= (write_back_en << 6);
    c |= (en << 7);
    writeReg(0x51, c);
    printf("OK\n");
}

static void setShadowRam() {
    printf("Enabling BIOS Shadow RAM...");
    //Copy BIOS contents to temp area
    for (int i = 0; i < 0x40000; i+=4) {
        *(unsigned int*)(0x100000 + i) = *(unsigned int*)(0xC0000 + i);
    }
    //We jump out to tmp area while setting up shadowing
    asm("jmp . + 0x40000 + 0x5");
    writeReg(0x52, 0xBF);
    for (int i = 0; i < 0x40000; i+=4) {
        *(unsigned int*)(0xC0000 + i) = *(unsigned int*)(0x100000 + i);
    }
    writeReg(0x52, 0xFF);
    //Jump back to shadow area
    asm("jmp . - 0x40000 + 0x5");
    printf("OK\n");
}
#define BIOS_SIZE_64K 0
#define BIOS_SIZE_128K 1
static void setBiosRomOptions(int cacheable, int size, int combine) {
    printf("Setting BIOS ROM Options (Cachable Shadow/Rom Size)...");
    char c = readReg(0x53);
    c &= 0x0F;
    c |= cacheable << 4;
    c |= cacheable << 5;
    c |= size << 7;
    c |= combine << 6;
    writeReg(0x53, c);
    printf("OK\n");
}

static void setDeturboEnable(int deturbo) {
    printf("Setting Turbo/Deturbo (Allow turbo switch)...");
    char c = readReg(0x53);
    c &= 0xFE;
    c |= !deturbo;
    writeReg(0x53, c);
    printf("OK\n");
}

static void setBusClockOptions(int spd, int b16_time, int b8_time) {
    printf("Setting Bus Speed options...");
    writeReg(0x60, spd << 4);
    writeReg(0x61, b16_time << 6 | b8_time << 4 | 0xF);
    printf("OK\n");
}

void chipset_init() {
    printf("Attempting to initialize Chipset SiS85C460\n");
    postCode(0xC1);
    detectDramConfig();
    postCode(0xC2);
    setShadowRam();
    postCode(0xC3);
    setRamSpeed(DRAM_FASTEST, CAS_1T);
    postCode(0xC4);
    setCPUCache(1, CACHE_128KB, 1, 0, 1, 1);
    postCode(0xC5);
    setBiosRomOptions(1, BIOS_SIZE_64K, 0);
    postCode(0xC6);
    setDeturboEnable(1);
    postCode(0xC7);
    setBusClockOptions(0x7, 0x3, 0x3);
    printf("Chipset Initialized\n");
}

bool chipset_has_rom_shadowing() {
  return false;
}

bool chipset_has_pci() {
  return false;
}

u32 chipset_pci_config_read(u32 base_config_ddress, u32 offset) {
  return 0;
}

void chipset_pci_config_write(u32 base_config_ddress, u32 offset, u32 value) {
}

u32 chipset_pci_config_address(u8 bus, u8 dev, u8 func, u8 offset) {
  return 0;
}

void chipset_shadow_rom_from_src(u32 address, u32 size, u32 src) {
}

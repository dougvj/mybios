#include "vga.h"
#include "output.h"
#include "runtime_init.h"
#include "postcode.h"
#include "post.h"
#include "chipset.h"
#include "ata.h"
#include "pci.h"
#include "serial.h"
#include "util.h"
#include "interrupts.h"

unsigned int data_init_sentinel = 0xdeadc0de;

void delay() {
    for (int i = 0; i < 1000; i++) {
        asm("nop");
    }
}

void find_boundary_signatures() {
    printf("Searching for other banks' signatures 0x%X\n", 0xDEADBEEF);
    int count = 0;
    unsigned int start = 0x0;
    do {
        if (*(unsigned int*)start == 0xDEADBEEF) {
            printf("Found boundary at 0x%X\n", start);
            count++;
        }
        start += 0x10000;
    } while (start != 0x0);
    printf("Found %d boundaries.\n", count);
}

void cmos_write(unsigned char addr, unsigned char value) {
  outb(0x70, addr);
  outb(0x71, value);
}

void cmos_write16(unsigned char addr, unsigned short value) {
  cmos_write(addr, value & 0xFF);
  cmos_write(addr + 1, value >> 8);
}

unsigned char cmos_read(unsigned char addr) {
  outb(0x70, addr);
  delay();
  return inb(0x71);
}

unsigned short cmos_read16(unsigned char addr) {
  unsigned short value = cmos_read(addr + 1);
  value <<= 8;
  value |= cmos_read(addr);
  return value;
}


void beep() {
  outb(0x61, inb(0x61) | 3);
  delay();
  outb(0x61, inb(0x61) & ~3);
}



void mode13h_test() {
    // Jump to lower ROM that is shadowed
    unsigned char* vram = (unsigned char*)0xA0000;
    for (int i = 0; i < 320 * 200; i++) {
        vram[i] = 0x0F;
    }
    unsigned int count = 0;
    while (1) {
        for (int i = 0; i < 320 * 200; i++) {
            count++;
            vram[i] = (count>>24) ^ (count>>16) ^ (count>>8) ^ count;
        }
    }
    printf("VGA mode 13h enabled\n");
}



void print_io(int port) {
    printf("Port 0x%X: 0x%X\n", port, inb(port));
}


int detect_shadowing(int address, int msg) {
    volatile unsigned int* check = (void*)address;
    int orig = *check;
    *check = 0x0;
    int result;
    if (*check == 0x0) {
      if (msg) {
        printf("ROM shadowing at %x enabled\n", address);
      }
      result = 1;
    } else {
      if (msg) {
        printf("ROM shadowing at %x disabled\n", address);
      }
      result = 0;
    }
    *check = orig;
    return result;
}

void chipset_explore();

void main() {
    serial_init(115200);
    postCode(0x11);
    chipset_init();
    postCode(0x12);
    initRuntime();
    postCode(0x13);
    vgaCls();
    vgaSetCursor(0, 0);
    printf("EklecTech BIOS %s. by Doug Johnson (%s) running backend %s\n", "v0.1apre", "2024", CHIPSET_NAME);
    postCode(0x14);
    if (data_init_sentinel != 0xdeadc0de) {
        printf("Runtime init failed! Possibly low RAM condition\n");
    }
    postCode(0x15);
    doPost();
    postCode(0x16);
    pci_configure();
    postCode(0x17);
    ataInit();
    postCode(0x18);
    //probe_io_ports();
    //find_boundary_signatures();
    //chipset_explore();
    //fuzz_io_port(0x22, 0x23, 0);
    postCode(0x19);
    //mode13h_test();
    detect_shadowing(0xFFFF0000, 1);
    detect_shadowing(0xF0000, 1);
    detect_shadowing(0xC0000, 1);
    detect_shadowing(0xE0000, 1);
    detect_shadowing(0xD0000, 1);
    detect_shadowing(0xB0000, 1);
    //void(*chipset_explore_lower_addr)(void) = shadowed_call(chipset_explore);
    //interrupts_init();
    //interrupts_enable_watchdog();
    //interrupts_enable();
    //chipset_explore_lower_addr();
    printf("Done\n");
    // See if we can get into vga 13h mode
    asm("cli");
    real_mode_int(&(real_mode_int_params) {
        .int_num = 0x10,
        .ax = 0x13,
    });
    asm("sti");
    void(*mode13h_test_lower_addr)(void) = shadowed_call(mode13h_test);
    mode13h_test_lower_addr();

    asm("hlt");
}


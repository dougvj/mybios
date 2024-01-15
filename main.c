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
  return inb(0x71);
}

unsigned short cmos_read16(unsigned char addr) {
  unsigned short value = cmos_read(addr + 1);
  value <<= 8;
  value |= cmos_read(addr);
  return value;
}

void probe_io_ports() {
  delay();
  printf("Probing I/O ports 0x0-0x3FF\n");
  for (unsigned int i = 0; i < 0x3FF; i++) {
    if ((i & 0xF) == 0) {
      vgaPutChar('.');
    }
    // Exclude VGA
    if (i >= 0x3B0 && i <= 0x3DF) {
      continue;
    }
    // Exclude serial
    if (i >= 0x3F8 && i <= 0x3FF) {
      continue;
    }
    // Write to cmos so we can resume execution
    delay();
    // Make sure we can read back what we wrote
    byte read = inb(i);
    if (inb(i) != 0xFF) {
      printf("\nIO port 0x%X read back 0x%X", i, read);
    }
  }
  printf("\nDone probing I/O ports\n");
}

void beep() {
  outb(0x61, inb(0x61) | 3);
  delay();
  outb(0x61, inb(0x61) & ~3);
}

void fuzz_io_port(unsigned short index_port, unsigned short data_port, int resume) {
  printf("Fuzzing I/O port idx: 0x%X data: %X\n", index_port, data_port);
  int last_fuzz_index = cmos_read(0x14);
  int last_fuzz_value = cmos_read(0x15);
  if (last_fuzz_index >= 0xFF || !resume){
    last_fuzz_index = 0;
    last_fuzz_value = 0;
  }else {
    printf("Last fuzz index was 0x%X and value was 0x%X\n", last_fuzz_index, last_fuzz_value);
  }
  int orig_perf = probeSpeed();
  printf("Performance: %d tics\n", orig_perf);
  int orig_mem = probeRam(0x100000, 0x400, 0);
  for (unsigned char i = last_fuzz_index; i < 0xFF; i++) {
    cmos_write(0x14, i);
    for (unsigned char j = last_fuzz_value+1; j < 0xFF; j++) {
      cmos_write(0x15, j);
      outb(index_port, i);
      for (int k = 0; k < 4; k++) {
        asm("nop");
      }
      printf("idx: 0x%X data: 0x%X read: 0x%X, 0x%X\n", i, j, inb(data_port), inb(data_port + 1));
      outb(data_port, j);
      outb(data_port + 1, j);
      /*
      int new_perf = probeSpeed();
      printf("Performance: %d tics\n", new_perf);
      if (orig_perf - new_perf > 10) {
        printf("Performance increased by %d tics\n", orig_perf - new_perf);
      } else if (new_perf - orig_perf > 10) {
        printf("Performance decrease by %d tics\n", new_perf - orig_perf);
      }*/
      int new_mem = probeRam(0x100000, 0x400, 0);
      if (orig_mem - new_mem > 0) {
        printf("Memory increased by %d KB\n", orig_mem - new_mem / 124);
        beep();
      } else if (new_mem - orig_mem > 0) {
        printf("Memory decreased by %d KB\n", new_mem - orig_mem / 1024);
        beep();
      }
    }
  }
  printf("Done fuzzing I/O port -> 0x%X and <- 0x%x\n", index_port, data_port);
}

int pseudarand = 0x29c32a91;
int rand() {
    pseudarand = ((pseudarand>>1) ^ 0xf39e94be) ^ ((pseudarand & 1) << 31);
    return pseudarand;
}


void mode13h_test() {
    // See if we can get into vga 13h mode
    real_mode_int(&(real_mode_int_params) {
        .interrupt = 0x10,
        .ax = 0x13,
    });
    // Write to video ram
    unsigned char* vram = (unsigned char*)0xA0000;
    for (int i = 0; i < 320 * 200; i++) {
        vram[i] = 0x0F;
    }
    // Randomly write to video ram
    //Randomly set the palette
    for (int i = 0; i < 256; i++) {
        outb(0x3C8, i);
        outb(0x3C9, rand() % 0xFF);
        outb(0x3C9, rand() % 0xFF);
        outb(0x3C9, rand() % 0xFF);
    }
    while (1) {
        for (int i = 0; i < 320 * 200; i++) {
            vram[i] = rand() & 0xFF;
        }
    }
    printf("VGA mode 13h enabled\n");
}

void print_io(int port) {
    printf("Port 0x%X: 0x%X\n", port, inb(port));
}


void main() {
    serial_init(115200);
    postCode(0x11);
    initRuntime();
    postCode(0x12);
    vgaSetCursor(6, 0);
    postCode(0x13);
    printf("EklecTech BIOS %s. Doug Johnson (%s). C Code Entry Point\n", "Alpha", "2024");
    postCode(0x14);
    chipset_init();
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
    asm("hlt");
}


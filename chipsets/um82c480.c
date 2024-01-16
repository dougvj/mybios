#include "io.h"
#include "output.h"
#include "post.h"
#include "postcode.h"
#include "util.h"

const char CHIPSET_NAME[] = "UM82C480";

#define DELAY 10

static void small_delay() { asm("nop"); }

static void writeReg(char reg, char byte) {
  outb(0x22, reg);
  small_delay();
  outb(0x23, byte);
}

static void writeReg2(char reg, char byte) {
  outb(0x22, reg);
  small_delay();
  outb(0x24, byte);
}

static char readReg(char reg) {
  outb(0x22, reg);
  small_delay();
  char d = inb(0x23);
  return d;
}

static char readReg2(char reg) {
  outb(0x22, reg);
  small_delay();
  return inb(0x24);
}

static int detectMirroring(unsigned int size) {
  for (unsigned int i = 0x100000; i < size; i += 0x80000) {
    unsigned int *probe = (unsigned int *)i;
    *probe = i;
  }
  for (unsigned int i = 0x100000; i < size; i += 0x80000) {
    unsigned int *probe = (unsigned int *)i;
    if (*probe != i) {
      return 1;
    }
  }
  return 0;
}

void large_delay() {
  for (int i = 0; i < 100000; i++) {
    asm("nop");
  }
}

extern void beep();
extern unsigned char cmos_read(unsigned char addr);
extern void cmos_write(unsigned char addr, unsigned char value);

// List of interesting bits:
// 0x81 bit 2 crahes and somehow triggers a ton (random writes?),
// 0x81 bit 6 crashes
// 0x92 bit 7 crashes
// 0x92 bit 0 corrupts RAM, possibly cache control? Sentinel didn't change
// 0x9f seems to remap RAM, bios shadowing?
// 0x9d shows more ram?
// 0x9a bit 0 enables 16m ram, crashes
// 0x9a bit 1 also 16 m?
// 0x9a bit 2 reset ram size to 2m
// 0x9a bit 3 reset ram size ?
// 0x9a 0x3F 32m
// 0x9a 0x4f 32m
// 0x9a 0x5f 32m
// 0x9a upper 4 bits seem to do nothing, lower is RAM size
//               Maybe timing is the upper bits? need to do perf tests
// 0x9b bit 1 detected shadowing at 0xF0000!
// 0x9b bit 4 detected shadowing at 0xe0000
// 0x9d bit 0 shadows 0xc0000-0xc3fff
// 0x9d bit 1 shadows 0xc4000-0xc7fff
// 0x9d bit 2 shadows 0xc8000-0xcbfff
// 0x9d bit 3 shadows 0xcc000-0xcffff
// 0x9d bit 4 shadows 0xd0000-0xd3fff
// 0x9d bit 5 shadows 0xd4000-0xd7fff
// 0x9d bit 6 shadows 0xd8000-0xdbfff
// 0x9d bit 7 shadows 0xdc000-0xdffff
// 0x9f bit 0, 1 corrupts ram probably by shifing it. Where to?

volatile unsigned int sentinel = 0xceaddead;
void toggle_all_bits(int reg, int mem_size) {
  int i = reg;
  int resume = cmos_read(0x16);
  printf("Sentinel location: %x\n", &sentinel);
  if (resume >= 0x8) {
    resume = 0;
  } else {
    printf("Bit toggle failure reg %x bit %d\n", reg, resume);
    resume++;
  }
  printf("%x\n", reg);
  for (int j = resume; j < 8; j++) {
    cmos_write(0x16, j);
    printf("%d", j);
    int toggle = 1 << j;
    int byte = readReg2(i);
    writeReg2(i, byte ^ toggle);
    if (sentinel != 0xceaddead) {
      printf(
          "\nSentinel changed stack is probably corrupted. Searching for it\n");
      for (int i = 0; i < 10; i++) {
        beep();
      }
      char *mem = 0;
      while (1) {
        if (mem[0] == 0xad && mem[1] == 0xde && mem[2] == 0xad &&
            mem[3] == 0xde) {
          printf("Found sentinel at %x. Halting\n", mem);
          break;
        }
        mem++;
      }
      asm("hlt");
    }
    int mem = probeAll(0x1000, 0x2000000, 0);
    if (mem != mem_size) {
      probeAll(0x1000, 0x2000000, 1);
      printf("\nRam changed %dKB -> %dKB\n", mem / 1024, mem_size / 1024);
      beep();
    }
    byte = readReg2(i);
    writeReg2(i, byte ^ toggle);
  }
  cmos_write(0x16, 8);
}

void try_all_combos(int reg, int mem_size) {
  int i = reg;
  int resume = cmos_read(0x18);
  printf("Sentinel location: %x\n", &sentinel);
  if (resume == 0xFF || resume == 0) {
    resume = 1;
  } else {
    printf("Failure on reg %x value %x\n", reg, resume);
    resume++;
  }
  int value = readReg2(i);
  printf("%x reads back %x\n", reg, value);
  for (int j = resume; j <= 0x7; j++) {
    cmos_write(0x18, j);
    printf("%x\n", j);
    writeReg2(i, j);
    if (sentinel != 0xceaddead) {
      // We have no stack! undo
      writeReg2(i, j - 1);
    }
    int mem = probeAll(0x400, 0x2000000, 1);
    if (mem != mem_size) {
      printf("\nRam changed %dKB -> %dKB\n", mem / 1024, mem_size / 1024);
      beep();
    }
  }
  cmos_write(0x18, 0);
}

void dump_regs() {
  for (int i = 0; i < 0x100; i++) {
    int byte = readReg2(i);
    if (byte != 0xff) {
      printf("Found reg at 0x%x (via reg 24): %x\n", i, byte);
    }
    //}
  }
}

void reset_resume() {
  cmos_write(0x14, 0);
  cmos_write(0x16, 0);
  cmos_write(0x18, 0);
}

void toggle_all(int mem_count) {
  int *mem = (void *)0xF0000;
  int resume = cmos_read(0x14);
  if (resume == 0xFF) {
    resume = 0;
  } else {
    printf("Resuming from 0x%x\n", resume);
  }
  for (int i = resume; i < 0x100; i++) {
    int byte = readReg2(i);
    if (byte != 0xff) {
      printf("Found reg at 0x%x (via reg 24): %x\n", i, byte);
      cmos_write(0x14, i);
      toggle_all_bits(i, mem_count);
    }
    //}
  }
  reset_resume();
}

void chipset_init() {
  // reset_resume();
  // volatile unsigned int *mem = (void *)0xF0000;
  // int mem_count = probeAll(0x400, 0x2000000, 1);

  // toggle_all_bits(0x80);
  // toggle_all_bits(0x81);
  // toggle_all_bits(0x92);
  // toggle_all_bits(0x97);
  // toggle_all_bits(0x9b);
  // try_all_combos(0x9a, mem_count);
  // Here are some hardcoded values that work for now
  register char best_config = 1;
  register int best_mem_count = 0;
  for (register char config = 0; config < 0x10; config++) {
    writeReg2(0x9a, config);
    postCode(config);
    // Write to 0x400 to make sure we can read back what we wrote
    volatile unsigned char *test = (void *)0x1000;
    *test = 0x55;
    if (*test != 0x55) {
      writeReg2(0x9a, best_config);
      continue;
    }
    int mem = probeRam(0x100000, 0x1000, 0);
    if (mem > best_mem_count) {
      if (detectMirroring(mem + 0x100000)) {
        continue;
      } else {
        // printf("Found config %x with %dKB\n", config, mem / 1024);
        best_mem_count = mem;
        best_config = config;
      }
    }
  }

  writeReg2(0x9a, best_config);
  memcpy((void *)0x60000, (void *)0xf0000, 0x10000);
  writeReg2(0x9b, 0x02); // Shadow 0xf0000
  memcpy((void *)0xf0000, (void *)0x60000, 0x10000);

}

bool chipset_has_rom_shadowing() { return false; }

bool chipset_has_pci() { return false; }

dword chipset_pci_config_read(dword base_config_ddress, dword offset) {
  return 0;
}

void chipset_pci_config_write(dword base_config_ddress, dword offset,
                              dword value) {}

void chipset_shadow_rom_from_src(dword src, dword dst, dword size) {}

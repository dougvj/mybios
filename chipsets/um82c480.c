#include "io.h"
#include "output.h"
#include "post.h"
#include "postcode.h"
#include "util.h"
#include "interrupts.h"
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

// 0x81 Reset value 0x08
// bit 0-2 crahes, 4, 5, do nothing, 6, 7 crash

// 0x91 bit 0 crashes
// 0x91 bit 1 abd 2 slightly lower memory performance
// Maybe bus wait states, noticed performance of i/o space changed
// 0x91 remaining seem to do nothing
// 0x91 bit 0 causes VGA writes to be wrong for some reason, like no
// characters are displayed but cursor updates
//
// 0x92 reset value is 0x10
// 0x92 bit 0 corrupts RAM access but sentinel is still OK. Got a segment
// not present fault at fff000103 for some reason
// The rest of the bits seem to do nothing
//
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

#define WRITE_SERIAL(x)                                                        \
  while(inb(0x3f8 + 5) & 0x20) {}                                               \
  outb(0x3f8, x)


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
  // Enable cache
  int cr0;
  asm("mov %%cr0, %0" : "=r"(cr0));
  printf("CR0: %x\n", cr0);
  // Enable internal cache
  cr0 &= ~0x60000000;
  // Flush cache
  asm("wbinvd");
  asm("mov %0, %%cr0" : : "r"(cr0));
  printf("CR0: %x\n", cr0);
  // Now not sure what this does but seems to increase performance:
  //
  //writeReg2(0x91, 0xAA);// Enable cache?
  asm("wbinvd");

}

int detect_shadowing(int address, int msg);

inline static void set_beep_freq(int freq) {
  outb(0x43, 0xb6);
  int divisor = 1193180 / freq;
  outb(0x42, divisor & 0xff);
  outb(0x42, divisor >> 8);
}

#define abs(x) ((x) < 0 ? -(x) : (x))

unsigned int sentinel = 0xceaddead;

// Assumes shadowing
void chipset_explore() {
  // Read initial values
  int regs[0xFF];
  int regs2[0xFF];
  for(int i = 0; i < 0xFF; i++) {
    regs[i] = readReg(i);
    regs2[i] = readReg2(i);
  }
  int (*probeSpeedLower)() = shadowed_call(probeSpeed);
  int (*probeMemSpeedLower)() = shadowed_call(probeMemSpeed);
  // OK let's try to toggle different bits and see if our performanc changes.
  //
  // This assumes already that we have dram configured and shadowing enabled

  int i = 0x70;
  int resume = 0xff;
  int toggle_mode = 1;
  //int resume = cmos_read(0x16);
  printf("Sentinel location: %x\n", &sentinel);
  int orig_speed = probeSpeedLower();
  printf("Orig Speed: %d\n", orig_speed);
  int mem_speed = probeMemSpeedLower();
  printf("Orig Mem Speed: %d\n", mem_speed);
  // Frequencies of major scale
  int freq_scale[] = {
      130, 146, 164, 174, 196, 220, 246, 261
  };
  if (resume >= 0xFF) {
    resume = 0;
  } else {
    printf("Failure reg %x val %d\n", i, resume);
    resume++;
  }
  printf("%x: %x\n", i, readReg2(i));
  for (int j = resume; j <= 0xFF; j++) {
    interrupts_watchdog_reset();
    if (toggle_mode && j > 0x7) {
      break;
    }
    //cmos_write(0x16, j);
    set_beep_freq(freq_scale[j&0x7] * 2);
    // Enable beep
    outb(0x61, inb(0x61) | 3);
    int byte = readReg2(i);
    int new_byte;
    if (toggle_mode) {
      int toggle = 1 << j;
      new_byte = byte ^ toggle;
    } else {
      new_byte = j;
    }
    printf("%d: %x\n", j, new_byte);
    // Flush cache just in case
    asm("wbinvd");
    writeReg2(i, new_byte);
    // Disable beep
    if (sentinel != 0xceaddead) {
      printf("Sentinel corrupted: %x\n", sentinel);
      //outb(0x61, inb(0x61) & ~3);
      char *mem = (char*)0x1000;
      while (1) {
        // enable beep
        set_beep_freq(((int)mem >> 20) + 100);
        outb(0x61, inb(0x61) | 3);
        if (mem[0] == 0xad && mem[1] == 0xde && mem[2] == 0xad &&
            mem[3] == 0xde) {
          // brief beep
          outb(0x61, inb(0x61) | 3);
          for (int i = 0; i < 100000; i++) {
            asm("nop");
          }
          break;
        }
        outb(0x61, inb(0x61) & ~3);
        mem +=4;
      }
      writeReg2(i, byte);
      printf("Found sentinel at %x which is a shift of %d\n", mem, mem - (char*)sentinel);
    } else {
      int new_speed = probeSpeedLower();
      printf("New speed: %d\n", new_speed);
      if (abs(new_speed - orig_speed) > 100) {
        printf("Bit %d is a performance change of %i\n", j, abs(new_speed - orig_speed));
      }
      int new_mem_speed = probeMemSpeed();
      printf("New mem speed: %d\n", new_mem_speed);
      if (abs(new_mem_speed - mem_speed) > 100) {
        printf("Bit %d is a mem performance change of %i\n", j, abs(new_mem_speed - mem_speed));
      }
      if (detect_shadowing(0xFFF00000, 0)) {
        printf("Bit %d is a shadowing change in upper ROM region\n", j);
      }
      // Probe all registers for changes
      for(int i = 0; i < 0xFF; i++) {
        int byte = readReg(i);
        if (byte != regs[i]) {
          printf("Found reg(23) %x changed from %x to %x\n", i, regs[i], byte);
          // regs[i] = byte;
        }
        byte = readReg2(i);
        if (byte != regs2[i]) {
          printf("Found reg(24) %x changed from %x to %x\n", i, regs2[i], byte);
          // regs2[i] = byte;
        }
      }
      outb(0x61, inb(0x61) & ~3);
      writeReg2(i, byte);
    }
  }
  /*for(;;) {
    //outb(0x61, inb(0x61) & ~3);
    int speed = probeMemSpeed();
    printf("Speed: %d\r", speed);
  }*/
}

bool chipset_has_rom_shadowing() { return false; }

bool chipset_has_pci() { return false; }

dword chipset_pci_config_read(dword base_config_ddress, dword offset) {
  return 0;
}

void chipset_pci_config_write(dword base_config_ddress, dword offset,
                              dword value) {}

void chipset_shadow_rom_from_src(dword src, dword dst, dword size) {}

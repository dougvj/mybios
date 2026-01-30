#include "interrupt.h"
#include "io.h"
#include "output.h"
#include "post.h"
#include "postcode.h"
#include "util.h"
const char CHIPSET_NAME[] = "UM82C480";

#define DELAY 10

static void small_delay() { asm("nop"); }

// This writes to the UM82C206. There is only one register which controls the
// wait states of accessing the PIC, PIT, etc which this chip implements
// BREAKTHROUGH: This chip is the same as the Opti 82C206
static void writeReg206(char reg, char u8) {
  outb(0x22, reg);
  small_delay();
  outb(0x23, u8);
}

static void writeReg480(char reg, char u8) {
  outb(0x22, reg);
  small_delay();
  outb(0x24, u8);
}

static char readReg206(char reg) {
  outb(0x22, reg);
  small_delay();
  char d = inb(0x23);
  return d;
}

static char readReg480(char reg) {
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

// Here are the known registers
// 0x80 Reset value 0x02, seems to be read only
// 0x81 Reset value 0x08
// 0x82 Reset value 0x00
// 0x91 Reset value 0xA8 bits 1-3 seem to affect memory performance, but fastest 0xE0 is unstable, crashes after a minutes or so
// 0x92 Reset value 0x10 Upper bits seem to affect memory performance, reset value is fastest. Bit 5 seems to be fixed
// 0x93 Reset value 0x00
// 0x94 Reset value 0xC0
// 0x95 Reset value 0x00
// 0x96 Reset value 0x00
// 0x97 Reset value 0x30
// 0x98 Reset value 0x00
// 0x99 Reset value 0x00
// 0x9A Reset value 0x01 (Known to be RAM size on bottom 4 bits)
// 0x9B Reset value 0x00 (BIOS shadowing on bit 1
// 0x9C Reset value 0x00
// 0x9D Reset value 0x41 (Fined grained shadowing from 0xc0000 to 0xdffff)

// 0x81 Reset value 0x08
// bit 0-2 crahes, 4, 5, do nothing, 6, 7 crash

// 0x91 bit 0 crashes. Fast reset?
// 0x91 bits, 1, 2, 3 affect memory performance. Bus access? Seems like 0xE0000
//                   is the fastest
// 0x91 remaining seem to do nothing and reset to 0xA0
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
// 0x9f bit 0, 1 corrupts ram probably by shifing it. Where to? I think this is
// the remap register

#define WRITE_SERIAL(x)                                                        \
  while (inb(0x3f8 + 5) & 0x20) {                                              \
  }                                                                            \
  outb(0x3f8, x)

void dump_regs() {
  for (int i = 0; i < 0x100; i++) {
    int u8 = readReg480(i);
    if (u8 != 0xff) {
      printf("Found reg at 0x%x (via reg 24): %x\n", i, u8);
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
    writeReg480(0x9a, config);
    postCode(config);
    // Write to 0x400 to make sure we can read back what we wrote
    volatile unsigned char *test = (void *)0x1000;
    *test = 0x55;
    if (*test != 0x55) {
      writeReg480(0x9a, best_config);
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
  writeReg480(0x9a, best_config);
  memcpy((void *)0x60000, (void *)0xf0000, 0x10000);
  writeReg480(0x9b, 0x02); // Shadow 0xf0000
  memcpy((void *)0xf0000, (void *)0x60000, 0x10000);
  // Enable cache
  int cr0, cr3;
  asm("mov %%cr0, %0" : "=r"(cr0));
  asm("mov %%cr3, %0" : "=r"(cr3));
  printf("CR0: %x\n", cr0);
  printf("CR3: %x\n", cr3);
  // Enable internal cache
  cr0 &= ~0x60000000;
  // Enable external cache
  cr3 |= 0x00000008;
  cr3 &= ~0x00000010;
  asm("wbinvd");
  asm("mov %0, %%cr3" : : "r"(cr3));
  asm("mov %0, %%cr0" : : "r"(cr0));
  printf("CR0: %x\n", cr0);
  // Not sure what this does. Bus states?
  writeReg480(0x91, 0x0E);
  writeReg480(0x92, 0xF0);
}

int detect_shadowing(int address, int msg);

#define REG 0x96
#define INTERVAL 500
#define STRIDE 0x1

static unsigned int* const prev_top = (unsigned int*)(0x1000);
static int prev_val = 0;
static int count = 0;

// After runtime init, internal state is valid
void chipset_post() {
  *prev_top = probeRam(0x100000, 0x1000, 0);
  unsigned int try_new_registers(unsigned int ticks) {
    printf("UM82C206: %d\n", readReg206(0x01));
    int reg = REG;
    if (prev_val == 0) {
      unsigned char val = readReg480(reg);
      prev_val = val;
    }
    int new_val = (prev_val + STRIDE) & 0xff;
    printf("Trying 0x%x: %x\n", reg, new_val);
    writeReg480(reg, new_val);
    u8 set_val = readReg480(reg);
    if (new_val != set_val) {
      printf("Failed to set 0x%x to %x, got %x\n", reg, new_val, set_val);
    }
    prev_val = new_val;
    // Was there a remap?
    unsigned int top = probeRam(0x100000, 0x1000, 0);
    if (top != *prev_top) {
      printf("Found remap from %x to %x\n", prev_top + 0x100000, top + 0x100000);
      *prev_top = top;
    }
    if (detect_shadowing(0xFFFF0000, 0)) {
      printf("Shadowing at FFFF0000\n");
    }
    count++;
    if (count > 256) {
      // Beep
      outb(0x61, inb(0x61) | 3);
      itr_disable();
      asm("hlt");
    }
    return ticks;
  }
  //s_register_timer_callback(try_new_registers, INTERVAL);
}


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
  for (int i = 0; i < 0xFF; i++) {
    regs[i] = readReg206(i);
    regs2[i] = readReg480(i);
  }
  // OK let's try to toggle different bits and see if our performanc changes.
  //
  // This assumes already that we have dram configured and shadowing enabled

  int i = 0x70;
  int resume = 0xff;
  int toggle_mode = 1;
  // int resume = cmos_read(0x16);
  printf("Sentinel location: %x\n", &sentinel);
  int orig_speed = probeSpeed();
  printf("Orig Speed: %d\n", orig_speed);
  int mem_speed = probeMemSpeed();
  printf("Orig Mem Speed: %d\n", mem_speed);
  // Frequencies of major scale
  int freq_scale[] = {130, 146, 164, 174, 196, 220, 246, 261};
  if (resume >= 0xFF) {
    resume = 0;
  } else {
    printf("Failure reg %x val %d\n", i, resume);
    resume++;
  }
  printf("%x: %x\n", i, readReg480(i));
  for (int j = resume; j <= 0xFF; j++) {
    if (toggle_mode && j > 0x7) {
      break;
    }
    // cmos_write(0x16, j);
    set_beep_freq(freq_scale[j & 0x7] * 2);
    // Enable beep
    outb(0x61, inb(0x61) | 3);
    int u8 = readReg480(i);
    int new_u8;
    if (toggle_mode) {
      int toggle = 1 << j;
      new_u8 = u8 ^ toggle;
    } else {
      new_u8 = j;
    }
    printf("%d: %x\n", j, new_u8);
    // Flush cache just in case
    asm("wbinvd");
    writeReg480(i, new_u8);
    // Disable beep
    if (sentinel != 0xceaddead) {
      printf("Sentinel corrupted: %x\n", sentinel);
      // outb(0x61, inb(0x61) & ~3);
      char *mem = (char *)0x1000;
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
        mem += 4;
      }
      writeReg480(i, u8);
      printf("Found sentinel at %x which is a shift of %d\n", mem,
             mem - (char *)sentinel);
    } else {
      int new_speed = probeSpeed();
      printf("New speed: %d\n", new_speed);
      if (abs(new_speed - orig_speed) > 100) {
        printf("Bit %d is a performance change of %i\n", j,
               abs(new_speed - orig_speed));
      }
      int new_mem_speed = probeMemSpeed();
      printf("New mem speed: %d\n", new_mem_speed);
      if (abs(new_mem_speed - mem_speed) > 100) {
        printf("Bit %d is a mem performance change of %i\n", j,
               abs(new_mem_speed - mem_speed));
      }
      if (detect_shadowing(0xFFF00000, 0)) {
        printf("Bit %d is a shadowing change in upper ROM region\n", j);
      }
      // Probe all registers for changes
      for (int i = 0; i < 0xFF; i++) {
        int u8 = readReg206(i);
        if (u8 != regs[i]) {
          printf("Found reg(23) %x changed from %x to %x\n", i, regs[i], u8);
          // regs[i] = u8;
        }
        u8 = readReg480(i);
        if (u8 != regs2[i]) {
          printf("Found reg(24) %x changed from %x to %x\n", i, regs2[i], u8);
          // regs2[i] = u8;
        }
      }
      outb(0x61, inb(0x61) & ~3);
      writeReg480(i, u8);
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

u32 chipset_pci_config_read(u32 base_config_ddress, u32 offset) {
  return 0;
}

void chipset_pci_config_write(u32 base_config_ddress, u32 offset,
                              u32 value) {}

void chipset_shadow_rom_from_src(u32 src, u32 dst, u32 size) {}

void chipset_fast_reset(void) {
  // Reset through KB controller
  printf("Resetting\n");
  outb(0x64, 0xFE);
}


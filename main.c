#include "ata.h"
#include "chipset.h"
#include "interrupts.h"
#include "output.h"
#include "pci.h"
#include "post.h"
#include "postcode.h"
#include "runtime_init.h"
#include "serial.h"
#include "util.h"
#include "vga.h"

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
    if (*(unsigned int *)start == 0xDEADBEEF) {
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
  unsigned char *vram = (unsigned char *)0xA0000;
  for (int i = 0; i < 320 * 200; i++) {
    vram[i] = 0x0F;
  }
  unsigned int count = 0;
  unsigned int frame = 0;
  unsigned int running_average = 0;
  unsigned int running_average_count = 0;
  int timer_id = -1;
  while (1) {
    int start_time = interrupts_timer_ticks();
    int off = 0;
    for (int i = 0; i < 200; i++) {
      for (int j = 0; j < 320; j++) {
        vram[off + j] = (i + j + count) & 0xFF;
      }
      off += 320;
    }
    count++;
    frame++;
    interrupts_watchdog_reset();
    int end_time = interrupts_timer_ticks();
    int time = end_time - start_time;
    running_average += time;
    running_average_count++;
    unsigned int print_running_average(unsigned int ticks) {
      if (running_average_count == 0) {
        return ticks * 2;
      }
      printf("Average frame time: %dms\n",
             running_average / running_average_count);
      running_average = 0;
      running_average_count = 0;
      return ticks;
    }
    if (timer_id < 0) {
      timer_id = interrupts_register_timer_callback(print_running_average, 500);
    }
  }
  printf("VGA mode 13h enabled\n");
}

void print_io(int port) { printf("Port 0x%X: 0x%X\n", port, inb(port)); }

int detect_shadowing(int address, int msg) {
  volatile unsigned int *check = (void *)address;
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

// TODO I think this is clobbering ram
void handover_from_hd(int offset) {
  printf("Loading to 0x80000 with soft handover to shadow rom at 0xF0000...\n");
  int read_sectors = 65536 / 512;
  char *buffer = (char *)0x80000;
  if (ataRead(0, offset, read_sectors, buffer) != 65536) {
    printf("Failed to read bootstrap\n");
    return;
  }
  for (int i = 0; i < 16; i++) {
    printf("%x ", buffer[i + 0xFFF0]);
  }
  interrupts_disable();
  // Soft reset doesn't work because we clobbered lower memory
  // TODO, we can probably load the bootstrap to 0x7C00,
  // setup a realmode routine to copy it to 0xF0000 and then do the
  // soft reset. Need to make sure the real mode routine also lives
  // outside of rom shadowing
  extern char soft_handover;
  extern char soft_handover_end;
  int soft_handover_size = (int)&soft_handover_end - (int)&soft_handover;
  // Copy soft handover to 0x70000
  char* ptr = (char*)&soft_handover;
  for (int i = 0; i < soft_handover_size; i++) {
    *(char *)(0x70000 + i) = ptr[i];
  }
  // Clear real mode IVT
  for (int i = 0; i < 256; i++) {
    *(char *)(0x0 + i) = 0;
  }
  // Clear text mode memory
  for (int i = 0; i < 0x8000; i++) {
    *(char *)(0xB8000 + i) = 0;
  }
  // Clear VGA memory
  for (int i = 0; i < 0x10000; i++) {
    *(char *)(0xA0000 + i) = 0;
  }
  // Now we can call it
  real_mode_call_params params = {
      .segment = 0x7000,
      .offset = 0x0,
  };
  real_mode_call(&params);
  printf("Soft handover failed");
}

int find_bootstrap_partition(char* MBR) {
  // Parse MBR
  int partition_offsets[4];
  int partition_sizes[4];
  char partition_types[4];
  for (int i = 0; i < 4; i++) {
    partition_offsets[i] = *(int *)(MBR + 0x1BE + i * 16 + 8);
    partition_sizes[i] = *(int *)(MBR + 0x1BE + i * 16 + 12);
    partition_types[i] = *(char *)(MBR + 0x1BE + i * 16 + 4);
  }
  int bootstrap_partition = -1;
  for (int i = 0; i < 4; i++) {
    if (partition_sizes[i] == 0) {
      continue;
    }
    printf("Partition %d: Offset 0x%X, Size 0x%X, Type 0x%X\n", i,
           partition_offsets[i], partition_sizes[i], partition_types[i]);
    if (partition_types[i] == 0x68) {
      bootstrap_partition = i;
    }
  }
  if (bootstrap_partition < 0) {
    printf("No bootstrap partition found\n");
    return -1;
  }
  return partition_offsets[bootstrap_partition];
}

bool boot_from_hd() {
  char* bootsector = (char*)0x7C00;
  if (ataRead(0, 0, 1, bootsector) != 512) {
    printf("Failed to read MBR\n");
    return false;
  }
  // Look for bootsector signature
  if (bootsector[510] != 0x55 || bootsector[511] != 0xAA) {
    printf("Invalid MBR signature\n");
    return false;
  }
  vgaCls();
  set_vga_enabled(0);
  // Call real mode routine to jump into bootsector
  real_mode_call_params params = {
      .segment = 0x0,
      .offset = 0x7C00,
      .dx = 0x0080,
  };
  real_mode_call(&params);
  printf("Boot from HD failed\n");
  return false;
}

bool oprom_exists(unsigned char *oprom) {
  if (oprom[0] != 0x55 || oprom[1] != 0xAA) {
    printf("Invalid oprom signature\n");
    return false;
  }
  return true;
}

bool oprom_checksum_valid(unsigned char *oprom) {
  unsigned int size = oprom[2] * 512;
  if (size > 0x40000) {
    return false;
  }
  unsigned int checksum = 0;
  for (unsigned int i = 0; i < size; i++) {
    checksum += oprom[i];
  }
  if ((checksum & 0xFF) == 0) {
    return true;
  } else {
    printf("Invalid oprom checksum\n");
    return false;
  }
}

bool oprom_valid(unsigned char *oprom) {
  return oprom_exists(oprom) && oprom_checksum_valid(oprom);
}

bool call_oprom(unsigned char *oprom) {
  real_mode_call_params params = {
      .segment = (int)oprom >> 4,
      .offset = 3,
  };
  real_mode_call(&params);
  return true;
}

void main() {
  // Init runtime after chipset init but we need to set callbacks TODO
#ifdef ENABLE_SERIAL
  // Enable serial port before runtime init
  serial_init(115200);
#endif
  postCode(0x11);
  chipset_init();
  postCode(0x12);
  initRuntime();
  // Enable again in case chipset/runtime init broke internal state
#ifdef ENABLE_SERIAL
  bool serial_enabled = serial_init(115200);
#endif
  if (oprom_valid((unsigned char *)0xC0000)) {
    printf("Found VGA BIOS, Calling\n");
    call_oprom((unsigned char *)0xC0000);
    set_vga_enabled(1);
    postCode(0x13);
    vgaCls();
  } else {
    printf("Could not find VGA BIOS\n");
    set_vga_enabled(0);
  }
  chipset_post();
  vgaSetCursor(0, 0);
  printf("EklecTech BIOS %s. by Doug Johnson (%s) running backend %s\n",
         "v0.1apre", "2024", CHIPSET_NAME);
#ifdef ENABLE_SERIAL
  if (serial_enabled) {
    printf("Serial port enabled, COM1 115200\n");
  } else {
    printf("Serial port not found\n");
    beep();
    while (1) {
      asm("hlt");
    }
  }
#endif
  postCode(0x14);
  if (data_init_sentinel != 0xdeadc0de) {
    printf("Runtime init failed! Possibly low RAM condition\n");
  }
  postCode(0x15);
  doPost();
  postCode(0x16);
#ifdef ENABLE_PCI
  pci_configure();
#endif
  postCode(0x17);
  interrupts_init();
  serial_set_buffered(1);
  postCode(0x18);
  ataInit();
  // probe_io_ports();
  // find_boundary_signatures();
  // chipset_explore();
  // fuzz_io_port(0x22, 0x23, 0);
  postCode(0x19);
  interrupts_enable();
  // mode13h_test();
  detect_shadowing(0xFFFF0000, 1);
  bool __attribute__((unused)) rom_region_writable =
    detect_shadowing(0xF0000, 1);
  detect_shadowing(0xC0000, 1);
  detect_shadowing(0xE0000, 1);
  detect_shadowing(0xD0000, 1);
  detect_shadowing(0xB0000, 1);
  // void(*chipset_explore_lower_addr)(void) = shadowed_call(chipset_explore);
  //  Note: interrupts break real_mode_int even when not enabled
  //  TODO
  // interrupts_enable_watchdog();
  // interrupts_enable_watchdog();
  // interrupts_enable();*/
  // chipset_explore_lower_addr();
  printf("Done\n");
  bool success = boot_from_hd();
  if (!success) {
    printf("Failed to boot from HD\n");
  }
  while (1) {
    asm("hlt");
  }
  // If we are here we failed to bootstrap from HD
  interrupts_disable();
  // See if we can get into vga 13h mode
  real_mode_int(&(real_mode_int_params){
      .int_num = 0x10,
      .ax = 0x13,
  });
  interrupts_enable();
  void (*mode13h_test_lower_addr)(void) = shadowed_call(mode13h_test);
  mode13h_test_lower_addr();

  asm("hlt");
}

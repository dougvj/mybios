#include "ata.h"
#include "chipset.h"
#include "interrupt.h"
#include "output.h"
#include "pci.h"
#include "post.h"
#include "postcode.h"
#include "runtime_init.h"
#include "serial.h"
#include "util.h"
#include "timer.h"
#include "vga.h"
#include "keyboard.h"
#include "bios.h"
#include "dev.h"
#include "bda.h"

u32 data_init_sentinel = 0xdeadc0de;

void delay() {
  for (int i = 0; i < 1000; i++) {
    asm("nop");
  }
}

void find_boundary_signatures() {
  printf("Searching for other banks' signatures 0x%X\n", 0xDEADBEEF);
  int count = 0;
  u32 start = 0x0;
  do {
    if (*(u32 *)start == 0xDEADBEEF) {
      printf("Found boundary at 0x%X\n", start);
      count++;
    }
    start += 0x10000;
  } while (start != 0x0);
  printf("Found %d boundaries.\n", count);
}

void cmos_write(u8 addr, u8 value) {
  outb(0x70, addr);
  outb(0x71, value);
}

void cmos_write16(u8 addr, u16 value) {
  cmos_write(addr, value & 0xFF);
  cmos_write(addr + 1, value >> 8);
}

u8 cmos_read(u8 addr) {
  outb(0x70, addr);
  delay();
  return inb(0x71);
}

u16 cmos_read16(u8 addr) {
  u16 value = cmos_read(addr + 1);
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
  u8 *vram = (u8 *)0xA0000;
  for (int i = 0; i < 320 * 200; i++) {
    vram[i] = 0x0F;
  }
  u32 count = 0;
  u32 frame = 0;
  u32 running_average = 0;
  u32 running_average_count = 0;
  int timer_id = -1;
  while (1) {
    int start_time = timer_get_ticks(dev_timer_primary);
    int off = 0;
    for (int i = 0; i < 200; i++) {
      for (int j = 0; j < 320; j++) {
        vram[off + j] = (i + j + count) & 0xFF;
      }
      off += 320;
    }
    count++;
    frame++;
    timer_watchdog_reset(dev_timer_primary);
    int end_time = timer_get_ticks(dev_timer_primary);
    int time = end_time - start_time;
    running_average += time;
    running_average_count++;
    u32 print_running_average(u32 ticks) {
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
      timer_id = timer_register_callback(dev_timer_primary, print_running_average, 500);
    }
  }
  printf("VGA mode 13h enabled\n");
}

void print_io(int port) { printf("Port 0x%X: 0x%X\n", port, inb(port)); }

int detect_shadowing(int address, int msg) {
  volatile u32 *check = (void *)address;
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
void handover_from_hd(int offset, ata_drive* drive) {
  printf("Loading to 0x80000 with soft handover to shadow rom at 0xF0000...\n");
  int read_sectors = 65536 / 512;
  char *buffer = (char *)0x80000;
  ata_drive_select(drive->dev, drive->flags & ATA_DRIVE_SLAVE);
  if (ata_read_lba(0, offset, read_sectors, buffer) != 65536) {
    printf("Failed to read bootstrap\n");
    return;
  }
  for (int i = 0; i < 16; i++) {
    printf("%x ", buffer[i + 0xFFF0]);
  }
  itr_disable();
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

void boot_from_hd(ata_drive* drive) {
  if (drive->dev) {
    ata_drive_select(drive->dev, drive->flags & ATA_DRIVE_SLAVE);
    msleep(2);
    char* bootsector = (char*)0x7C00;
    if (ata_read_lba(drive->dev, 0, 1, bootsector) != 512) {
      printf("Failed to read MBR\n");
      return;
    }
    // Look for bootsector signature
    if (bootsector[510] != 0x55 || bootsector[511] != 0xAA) {
      printf("Invalid MBR signature\n");
      return;
    }
    // Make this drive the boot drive
    // TODO
    dev_ata_drives[0] = *drive;
    printf("Booting in 2 seconds...\n");
    msleep(2000);
    printf("Booting...!");
    vgaCls();
    itr_setup_real_mode();
    set_vga_enabled(0);
    // Call real mode routine to jump into bootsector
    real_mode_call_params params = {
        .segment = 0x0,
        .offset = 0x7C00,
        .dx = 0x0080,
    };
    real_mode_call(&params);
    printf("Boot from HD failed\n");
  }
}

bool oprom_exists(u8 *oprom) {
  if (oprom[0] != 0x55 || oprom[1] != 0xAA) {
    //printf("Invalid oprom signature\n");
    return false;
  }
  return true;
}

bool oprom_checksum_valid(u8 *oprom) {
  u32 size = oprom[2] * 512;
  if (size > 0x40000) {
    return false;
  }
  u32 checksum = 0;
  for (u32 i = 0; i < size; i++) {
    checksum += oprom[i];
  }
  if ((checksum & 0xFF) == 0) {
    return true;
  } else {
    printf("Invalid oprom checksum\n");
    return false;
  }
}

bool oprom_valid(u8 *oprom) {
  return oprom_exists(oprom) && oprom_checksum_valid(oprom);
}

bool call_oprom(u8 *oprom) {
  real_mode_call_params params = {
      .segment = (int)oprom >> 4,
      .offset = 3,
  };
  real_mode_call(&params);
  return true;
}


dev_ata* dev_ata_primary;
dev_ata* dev_ata_secondary;
void hd_init() {
  int num_drives = 0;
  ata_drive* drive = (&dev_ata_drives[0]);
  void scan_dev(dev_ata* ata) {
    printf("Master: ");
    if (ata_identify(ata, false, drive)) {
      drive = (&dev_ata_drives[++num_drives]);
    }
    printf("Slave: ");
    if (ata_identify(ata, true, drive)) {
      drive = (&dev_ata_drives[++num_drives]);
    }
  }
  printf("Scanning primary ATA Channel...");
  dev_ata_primary = ata_init(0x1F0, IRQ14);
  if (dev_ata_primary) {
    printf(" Found\n");
    scan_dev(dev_ata_primary);
  } else {
    printf(" Not found\n");
  }
  printf("Scanning secondary ATA Channel...");
  dev_ata_secondary = ata_init(0x170, IRQ15);
  if (dev_ata_secondary) {
    printf(" Found\n");
    scan_dev(dev_ata_secondary);
  } else {
    printf(" Not found\n");
  }
}

void main() {
  postCode(0x10);
  itr_disable();
  // Init runtime after chipset init but we need to set callbacks TODO
#ifdef ENABLE_SERIAL
  // Enable serial port before runtime init
  serial_init(115200);
#endif
  printf("Hello, World!\n");
  postCode(0x11);
  void(*chipset_init_safe)(void) = unshadowed_call(chipset_init);
  chipset_init_safe();
  postCode(0x12);
  initRuntime();
  // Enable again in case chipset/runtime init broke internal state
#ifdef ENABLE_SERIAL
  bool serial_enabled = serial_init(115200);
#endif
  chipset_post();
#ifdef ENABLE_PCI
  pci_configure();
#endif
  if (oprom_valid((u8 *)0xC0000)) {
    printf("Found VGA BIOS, Calling\n");
    call_oprom((u8 *)0xC0000);
    set_vga_enabled(1);
    postCode(0x13);
    vgaCls();
  } else {
    printf("Could not find VGA BIOS\n");
    set_vga_enabled(0);
  }
  vgaSetCursor(0, 0);
  printf("EklecTech BIOS %s. by Doug Johnson (%s) running backend %s\n",
         "v0.1a", "2024", CHIPSET_NAME);
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
  itr_init();
  postCode(0x17);
  dev_timer_primary = timer_init(0x40, IRQ0);
  u32 inc_ticks(u32 ticks) {
    bda.irq0_counter += ticks;
    return ticks;
  }
  timer_register_callback(dev_timer_primary, inc_ticks, 1);
  serial_set_buffered(1);
  postCode(0x18);
  keyboard_init();
  postCode(0x19);
  hd_init();
  postCode(0x1A);
  bios_init();
  postCode(0x1B);
  itr_enable();
  postCode(0x1C);
  // mode13h_test();
  detect_shadowing(0xFFFF0000, 1);
  bool unused rom_region_writable =
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
  postCode(0x1D);
  printf("Done\n");
  for (int i = 0; i < 4; i++) {
    boot_from_hd(&dev_ata_drives[i]);
  }
  printf("Failed to boot\n");
  while (1) {
    asm("hlt");
  }
  // If we are here we failed to bootstrap from HD
  itr_disable();
  // See if we can get into vga 13h mode
  real_mode_int(&(real_mode_int_params){
      .int_num = 0x10,
      .ax = 0x13,
  });
  itr_enable();
  mode13h_test();

  asm("hlt");
}

#include "output.h"
#include "types.h"

char CHIPSET_NAME[] = "NULL";

void chipset_init() {
  printf("%s", "Null chipset declared, no driver loaded\n");
}

void chipset_post() {
}

bool chipset_has_rom_shadowing() { return false; }

bool chipset_has_pci() { return false; }

void chipset_shadow_rom_from_src(u32 address, u32 size, u32 src) {}

u32 chipset_pci_config_address(u8 bus, u8 dev, u8 func, u8 offset) {
  return 0xFFFFFFFF;
}


void chipset_pci_config_read(u32 base_config_address, u8 offset) {}

u32 chipset_pci_config_write(u32 base_config_address, u8 offset,
                               u32 data) {
  return 0xFFFFFFFF;
}

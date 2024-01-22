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

void chipset_shadow_rom_from_src(dword address, dword size, dword src) {}

dword chipset_pci_config_address(byte bus, byte dev, byte func, byte offset) {
  return 0xFFFFFFFF;
}


void chipset_pci_config_read(dword base_config_address, byte offset) {}

dword chipset_pci_config_write(dword base_config_address, byte offset,
                               dword data) {
  return 0xFFFFFFFF;
}

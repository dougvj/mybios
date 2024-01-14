#ifndef CHIPSET_H
#define CHIPSET_H
#include "types.h"
void chipset_init();
bool chipset_has_rom_shadowing();
void chipset_shadow_rom(dword address, dword len);
void chipset_shadow_rom_from_src(dword addres, dword len, dword src);
bool chipset_has_pci();
dword chipset_pci_config_address(byte bus, byte device, byte function, byte offset);
dword chipset_pci_config_read(dword base_config_address, byte offset);
void chipset_pci_config_write(dword base_config_address, byte offset, dword val);

#endif

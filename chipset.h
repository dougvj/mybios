#ifndef CHIPSET_H
#define CHIPSET_H
#include "types.h"
extern char CHIPSET_NAME[];
void chipset_init(void);
void chipset_post(void);
bool chipset_has_rom_shadowing(void);
void chipset_shadow_rom(dword address, dword len);
void chipset_shadow_rom_from_src(dword addres, dword len, dword src);
#ifdef ENABLE_PCI
bool chipset_has_pci(void);
dword chipset_pci_config_address(byte bus, byte device, byte function, byte offset);
dword chipset_pci_config_read(dword base_config_address, byte offset);
void chipset_pci_config_write(dword base_config_address, byte offset, dword val);
#endif
void chipset_fast_reset(void);

#endif

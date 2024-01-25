#ifndef CHIPSET_H
#define CHIPSET_H
#include "types.h"
extern char CHIPSET_NAME[];
void chipset_init(void);
void chipset_post(void);
bool chipset_has_rom_shadowing(void);
bool chipset_shadow_rom(u32 address, u32 len);
bool chipset_shadow_rom_from_src(u32 addres, u32 len, u32 src);
#ifdef ENABLE_PCI
bool chipset_has_pci(void);
u32 chipset_pci_config_address(u8 bus, u8 device, u8 function, u8 offset);
u32 chipset_pci_config_read(u32 base_config_address, u8 offset);
void chipset_pci_config_write(u32 base_config_address, u8 offset, u32 val);
#endif
void chipset_fast_reset(void);

#endif

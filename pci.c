#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#include "pci.h"
#include "chipset.h"
#include "debug.h"
#include "io.h"
#include "output.h"
#include "types.h"
#include "util.h"

#ifdef PCI_ENABLED

dword pci_type2_config_address(byte bus, byte device, byte func, byte offset) {
  dword address = 0x1 << 31 | (bus & 0x7F) << 16 | (device & 0x1F) << 11 |
                  (func & 0x7) << 8 | (offset & 0xFC);
  return address;
}

dword pci_type2_config_read(dword base_config_address, byte offset) {
  base_config_address = offset & 0xFC | (base_config_address & 0xFFFFFF00);
  outl(PCI_CONFIG_ADDRESS, base_config_address);
  return inl(PCI_CONFIG_DATA);
}

void pci_type2_config_write(dword base_config_address, byte offset,
                            dword data) {
  base_config_address = offset & 0xFC | (base_config_address & 0xFFFFFF00);
  outl(PCI_CONFIG_ADDRESS, base_config_address);
  outl(PCI_CONFIG_DATA, data);
}

dword pci_config_read(pci_device *dev, byte offset) {
  return chipset_pci_config_read(dev->base_config_address, offset);
}

void pci_config_write(pci_device *dev, byte offset, dword data) {
  chipset_pci_config_write(dev->base_config_address, offset, data);
}

dword query_bar_size(pci_device *dev, byte offset) {
  dword original = pci_config_read(dev, offset);
  pci_config_write(dev, offset, 0xFFFFFFFF);
  dword size = pci_config_read(dev, offset) & 0xFFFFFFFC;
  pci_config_write(dev, offset, original);
  return ~size + 1;
}

pci_device *query_pci_device(byte bus, byte device, pci_device *dev) {
  dword base_config_address = chipset_pci_config_address(bus, device, 0, 0);
  dword device_and_vendor = chipset_pci_config_read(base_config_address, 0);
  if (device_and_vendor == 0xFFFFFFFF) {
    return NULL;
  }
  dev->base_config_address = base_config_address;
  dev->bus = bus;
  dev->func = 0; // TODO
  dev->device = device;
  dev->vendor_id = device_and_vendor & 0xFFFF;
  dev->device_id = device_and_vendor >> 16;
  dword status_and_command = pci_config_read(dev, 4);
  dev->status = status_and_command & 0xFFFF;
  dev->command = status_and_command >> 16;
  dword class_and_revision = pci_config_read(dev, 8);
  dev->class = class_and_revision >> 24;
  dev->subclass = (class_and_revision >> 16) & 0xFF;
  dev->prog_if = (class_and_revision >> 8) & 0xFF;
  dev->revision = class_and_revision & 0xFF;
  dword reg_3 = pci_config_read(dev, 0xC);
  dev->bist = reg_3 >> 24;
  dev->header_type = (reg_3 >> 16) & 0xFF;
  dev->latency_timer = (reg_3 >> 8) & 0xFF;
  dev->cache_line_size = reg_3 & 0xFF;
  return dev;
};

void pci_map_bar(pci_device *dev, byte bar, dword address) {
  pci_config_write(dev, bar, address | 0x1);
}

dword pci_get_mapping(pci_device *dev, byte bar) {
  return pci_config_read(dev, bar) & 0xFFFFFFFC;
}

void pci_enable_bus_mastering(pci_device *dev) {
  dword command = pci_config_read(dev, 4);
  command |= 0x4;
  pci_config_write(dev, 4, command);
}

void pci_enable_io(pci_device *dev) {
  dword command = pci_config_read(dev, 4);
  command |= 0x1;
  pci_config_write(dev, 4, command);
}

void pci_enable_memory(pci_device *dev) {
  dword command = pci_config_read(dev, 4);
  command |= 0x2;
  pci_config_write(dev, 4, command);
}

pci_device dev;

pci_device *pci_get_device(byte bus, byte device) {
  if (query_pci_device(bus, device, &dev) != NULL) {
    return &dev;
  }
  return NULL;
}

pci_device *pci_find_device(word vendor_id, word device_id, byte index) {
  byte bus = 0;
  byte device = 0;
  byte count = 0;
  while (bus < 256) {
    while (device < 32) {
      pci_device *dev = pci_get_device(bus, device);
      if (dev == NULL) {
        return NULL;
      }
      if (dev->vendor_id == vendor_id && dev->device_id == device_id) {
        if (count == index) {
          return dev;
        }
        count++;
      }
      device++;
    }
    device = 0;
    bus++;
  }
  return NULL;
}

void pci_configure() {
  if (!chipset_has_pci()) {
    return;
  }
  printf("Enumerating PCI Bus\n");
  for (byte bus = 0; bus <= 127; bus++) {
    for (byte device = 0; device <= 31; device++) {
      pci_device *dev = pci_get_device(bus, device);
      if (dev != NULL) {
        printf("Found PCI Device: %X:%X\n", dev->vendor_id, dev->device_id);
        if (dev->class == PCI_CLASS_DISPLAY) {
          if (dev->subclass == 0x0) {
            dword current_address =
                pci_get_mapping(dev, PCI_CONFIG_EXPANSION_ROM_BASE);
            if (current_address != 0) {
              printf("Found VGA ROM at %X\n", current_address);
            } else {
              dword size = query_bar_size(dev, PCI_CONFIG_EXPANSION_ROM_BASE);
#ifdef BOCHS
              dword addr = 0xF00C0000;
#else
              dword addr = 0xC0000;
#endif
              printf("Found unmapped VGA Controller, mapping ROM of size %d to "
                     "%X\n",
                     size, addr);
              pci_map_bar(dev, PCI_CONFIG_EXPANSION_ROM_BASE, addr);
              pci_enable_bus_mastering(dev);
              pci_enable_io(dev);
              pci_enable_memory(dev);
              if (chipset_has_rom_shadowing()) {
                // chipset_shadow_rom_from_src(0xC0000, size, addr);
              }
              if (*(byte *)(0xC0000) == 0x55 && *(byte *)(0xC0001) == 0xAA) {
                printf("Option ROM is present, calling\n");
                printf("addr: %X\n",
                       pci_get_mapping(dev, PCI_CONFIG_EXPANSION_ROM_BASE));
                {
                  real_mode_call_params params = {
                      .segment = 0xC000,
                      .offset = 0x3,
                  };
                  real_mode_call(&params);
                }
                {
                  printf("Wow, done with real mode\n");
                  real_mode_int_params params = {
                      .interrupt = 0x10,
                      .ax = 0x0F00,
                  };
                  real_mode_int(&params);
                  printf("Current video mode: %u cols, mode %X\n",
                         params.ax >> 8, params.ax & 0xFF);
                  params.interrupt = 0x10;
                  params.ax = 0x03;
                  params.bx = 0;
                  real_mode_int(&params);
                  params.interrupt = 0x10;
                  params.ax = 0x0F00;
                  real_mode_int(&params);
                  vgaCls();
                  printf("Current video mode: %u cols, mode %X\n",
                         params.ax >> 8, params.ax & 0xFF);
                }
              } else {
                printf("Option ROM is not present\n");
              }
            }
          }
        }
      }
    }
  }
}

void pci_device_free(pci_device *dev) {
  // TODO
}

#else // PCI_ENABLED

void pci_configure() {
}

#endif // PCI_ENABLED

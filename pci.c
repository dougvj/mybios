#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#include "pci.h"
#include "chipset.h"
#include "debug.h"
#include "io.h"
#include "output.h"
#include "types.h"
#include "util.h"

#ifdef ENABLE_PCI

u32 pci_type2_config_address(u8 bus, u8 device, u8 func, u8 offset) {
  u32 address = 0x1 << 31 | (bus & 0x7F) << 16 | (device & 0x1F) << 11 |
                  (func & 0x7) << 8 | (offset & 0xFC);
  return address;
}

u32 pci_type2_config_read(u32 base_config_address, u8 offset) {
  base_config_address = (offset & 0xFC) | (base_config_address & 0xFFFFFF00);
  outl(PCI_CONFIG_ADDRESS, base_config_address);
  return inl(PCI_CONFIG_DATA);
}

void pci_type2_config_write(u32 base_config_address, u8 offset,
                            u32 data) {
  base_config_address = (offset & 0xFC) | (base_config_address & 0xFFFFFF00);
  outl(PCI_CONFIG_ADDRESS, base_config_address);
  outl(PCI_CONFIG_DATA, data);
}

u32 pci_config_read(pci_device *dev, u8 offset) {
#ifdef DEBUG
  if (offset & 0x3) {
    printf("Unaligned pci config access: 0x%x\n", (u32)offset);
    while(1) {
      asm("hlt");
    }
  }
  printf("pci_config_read(0x%x, 0x%x)", dev->base_config_address, offset);
#endif
  u32 ret = chipset_pci_config_read(dev->base_config_address, offset);
#ifdef DEBUG
  printf(" = 0x%x\n", ret);
#endif
  return ret;
}

void pci_config_write(pci_device *dev, u8 offset, u32 data) {
#ifdef DEBUG
  if (offset & 0x3) {
    printf("Unaligned pci config access: %x\n", (u32)offset);
    while(1) {
      asm("hlt");
    }
  }
  printf("pci_config_write(0x%x, 0x%x, 0x%x)\n", dev->base_config_address, offset, data);
#endif
  chipset_pci_config_write(dev->base_config_address, offset, data);
}

u16 pci_config_readw(pci_device* dev, u8 offset) {
  bool which_word = !!(offset & 0x2);
  u32 config = pci_config_read(dev, offset & 0xFC);
  return config >> (which_word * 16);
}

void pci_config_writew(pci_device* dev, u8 offset, u16 data) {
  bool which_word = !!(offset & 0x2);
  u32 config = pci_config_read(dev, offset & 0xFC);
  u32 mask = ~(0xFFFF << (which_word * 16));
  u32 ddata = data << (which_word * 16);
  pci_config_write(dev, offset & 0xFC, (config & mask) | ddata);
}

u8 pci_config_readb(pci_device* dev, u8 offset) {
  int which_byte = offset & 0x3;
  u32 config = pci_config_read(dev, offset & 0xFC);
  int shift = (which_byte * 8);
  return (config) >> shift;
}

void pci_config_writeb(pci_device* dev, u8 offset, u8 data) {
  int which_byte = offset & 0x3;
  u32 config = pci_config_read(dev, offset & 0xFC);
  int shift = (which_byte * 8);
  u32 mask = ~(0xFF << shift);
  config = (config & mask) | (((u32)data) << shift);
  pci_config_write(dev, offset & 0xFC, config);
}


u32 query_bar_size(pci_device *dev, u8 offset) {
  u32 original = pci_config_read(dev, offset);
  pci_config_write(dev, offset, 0xFFFFFFFF);
  u32 size = pci_config_read(dev, offset) & 0xFFFFFFF0;
  pci_config_write(dev, offset, original);
  return ~size + 1;
}

pci_device *query_pci_device(u8 bus, u8 device, pci_device *dev) {
  u32 base_config_address = chipset_pci_config_address(bus, device, 0, 0);
  u32 device_and_vendor = chipset_pci_config_read(base_config_address, 0);
  if (device_and_vendor == 0xFFFFFFFF) {
    return NULL;
  }
  dev->base_config_address = base_config_address;
  dev->bus = bus;
  dev->func = 0; // TODO
  dev->device = device;
  dev->vendor_id = device_and_vendor & 0xFFFF;
  dev->device_id = device_and_vendor >> 16;
  u32 status_and_command = pci_config_read(dev, 4);
  dev->status = status_and_command & 0xFFFF;
  dev->command = status_and_command >> 16;
  u32 class_and_revision = pci_config_read(dev, 8);
  dev->class = class_and_revision >> 24;
  dev->subclass = (class_and_revision >> 16) & 0xFF;
  dev->prog_if = (class_and_revision >> 8) & 0xFF;
  dev->revision = class_and_revision & 0xFF;
  u32 reg_3 = pci_config_read(dev, 0xC);
  dev->bist = reg_3 >> 24;
  dev->header_type = (reg_3 >> 16) & 0xFF;
  dev->latency_timer = (reg_3 >> 8) & 0xFF;
  dev->cache_line_size = reg_3 & 0xFF;
  return dev;
};

void pci_map_bar(pci_device *dev, u8 bar, u32 address) {
  u32 cur_address = pci_config_read(dev, bar);
  address &= 0xFFFFFFF0 | (cur_address & 0xF);
  if (bar == PCI_CONFIG_EXPANSION_ROM_BASE) {
    // enable bit
    address |= 1;
    // disable prefetch
    address &= ~0x8;
  }
  pci_config_write(dev, bar, address);
}

void pci_unmap_bar(pci_device *dev, u8 bar) {
  pci_config_write(dev, bar, 0);
}


u32 pci_get_mapping(pci_device *dev, u8 bar) {
  return pci_config_read(dev, bar) & 0xFFFFFFFC;
}

void pci_enable_bus_mastering(pci_device *dev) {
  u32 command = pci_config_read(dev, 4);
  command |= 0x4;
  pci_config_write(dev, 4, command);
}

void pci_enable_io(pci_device *dev) {
  u32 command = pci_config_read(dev, 4);
  command |= 0x1;
  pci_config_write(dev, 4, command);
}

void pci_enable_memory(pci_device *dev) {
  u32 command = pci_config_read(dev, 4);
  command |= 0x2;
  pci_config_write(dev, 4, command);
}

pci_device dev;

pci_device *pci_get_device(u8 bus, u8 device) {
  if (query_pci_device(bus, device, &dev) != NULL) {
    return &dev;
  }
  return NULL;
}

pci_device *pci_find_device(u16 vendor_id, u16 device_id, u8 index) {
  short bus = 0;
  u8 device = 0;
  u8 count = 0;
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
  for (u8 bus = 0; bus <= 127; bus++) {
    for (u8 device = 0; device <= 31; device++) {
      pci_device *dev = pci_get_device(bus, device);
      if (dev != NULL) {
        printf("Found PCI Device: %X:%X\n", dev->vendor_id, dev->device_id);
        u32 bar_sizes[7];
        int i = 0;
        for (int bar = PCI_CONFIG_BAR0;  bar <= PCI_CONFIG_BAR5; bar+=4) {
          bar_sizes[i++] = query_bar_size(dev, bar);
        }
        bar_sizes[6] = query_bar_size(dev, PCI_CONFIG_EXPANSION_ROM_BASE);
        for (int i = 0; i < 7; i++) {
          if (bar_sizes[i] != 0) {
            printf("BAR%i Size: 0x%x\n", i, bar_sizes[i]);
          }
        }
        if (dev->class == PCI_CLASS_DISPLAY) {
          if (dev->subclass == 0x0) {
            u32 addr =
                pci_get_mapping(dev, PCI_CONFIG_EXPANSION_ROM_BASE);
            if (addr != 0) {
              printf("Found VGA ROM at %X\n", addr);
            } else {
              // note that we have to map from the high address
              // because bOCHs complains if we map from 0xC0000
              if (chipset_has_rom_shadowing()) {
                addr = 0xEFFC0000;
              } else {
                addr = 0xC0000;
              }
              u32 size = query_bar_size(dev, PCI_CONFIG_EXPANSION_ROM_BASE);
              printf("Found unmapped VGA Controller, mapping ROM of size %d to "
                     "%X\n",
                     size, addr);
              pci_map_bar(dev, PCI_CONFIG_EXPANSION_ROM_BASE, addr);
              pci_enable_bus_mastering(dev);
              pci_enable_io(dev);
              pci_enable_memory(dev);
              if (chipset_has_rom_shadowing()) {
                chipset_shadow_rom_from_src(0xC0000, size, addr);
                pci_unmap_bar(dev, PCI_CONFIG_EXPANSION_ROM_BASE);
              }
              return;
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

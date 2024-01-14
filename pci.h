#ifndef PCI_H
#define PCI_H
#include "output.h"
#include "types.h"

dword pci_type2_config_address(byte bus, byte device, byte function, byte offset);
dword pci_type2_config_read(dword base_config_address, byte offset);
void pci_type2_config_write(dword base_config_address, byte offset, dword data);

typedef struct {
  dword base_config_address;
  byte bus;
  byte device;
  byte func;
  word vendor_id;
  word device_id;
  word status;
  word command;
  byte class;
  byte subclass;
  byte prog_if;
  byte revision;
  byte bist;
  byte header_type;
  byte latency_timer;
  byte cache_line_size;
  dword bar_size;
} pci_device;

enum pci_header_type {
  PCI_HEADER_TYPE_DEVICE = 0,
  PCI_HEADER_TYPE_BRIDGE = 1,
  PCI_HEADER_TYPE_CARDBUS = 2,
};

enum pci_class {
  PCI_CLASS_MASS_STORAGE = 0x01,
  PCI_CLASS_NETWORK = 0x02,
  PCI_CLASS_DISPLAY = 0x03,
  PCI_CLASS_MULTIMEDIA = 0x04,
  PCI_CLASS_MEMORY = 0x05,
  PCI_CLASS_BRIDGE = 0x06,
  PCI_CLASS_COMMUNICATION = 0x07,
  PCI_CLASS_SYSTEM = 0x08,
  PCI_CLASS_INPUT = 0x09,
  PCI_CLASS_DOCKING_STATION = 0x0A,
  PCI_CLASS_PROCESSOR = 0x0B,
  PCI_CLASS_SERIAL_BUS = 0x0C,
  PCI_CLASS_WIRELESS = 0x0D,
  PCI_CLASS_INTELLIGENT_IO = 0x0E,
  PCI_CLASS_SATELLITE = 0x0F,
  PCI_CLASS_ENCRYPTION = 0x10,
  PCI_CLASS_SIGNAL_PROCESSING = 0x11,
  PCI_CLASS_UNDEFINED = 0xFF,
};

enum pci_bar_type {
  PCI_BAR_TYPE_IO = 0,
  PCI_BAR_TYPE_MEMORY = 1,
};

enum pci_config_registers {
  PCI_CONFIG_VENDOR_ID = 0x00,
  PCI_CONFIG_DEVICE_ID = 0x02,
  PCI_CONFIG_COMMAND = 0x04,
  PCI_CONFIG_STATUS = 0x06,
  PCI_CONFIG_REVISION_ID = 0x08,
  PCI_CONFIG_PROG_IF = 0x09,
  PCI_CONFIG_SUBCLASS = 0x0A,
  PCI_CONFIG_CLASS = 0x0B,
  PCI_CONFIG_CACHE_LINE_SIZE = 0x0C,
  PCI_CONFIG_LATENCY_TIMER = 0x0D,
  PCI_CONFIG_HEADER_TYPE = 0x0E,
  PCI_CONFIG_BIST = 0x0F,
  PCI_CONFIG_BAR0 = 0x10,
  PCI_CONFIG_BAR1 = 0x14,
  PCI_CONFIG_BAR2 = 0x18,
  PCI_CONFIG_BAR3 = 0x1C,
  PCI_CONFIG_BAR4 = 0x20,
  PCI_CONFIG_BAR5 = 0x24,
  PCI_CONFIG_CARDBUS_CIS = 0x28,
  PCI_CONFIG_SUBSYSTEM_VENDOR_ID = 0x2C,
  PCI_CONFIG_SUBSYSTEM_ID = 0x2E,
  PCI_CONFIG_EXPANSION_ROM_BASE = 0x30,
  PCI_CONFIG_CAPABILITIES_POINTER = 0x34,
  PCI_CONFIG_INTERRUPT_LINE = 0x3C,
  PCI_CONFIG_INTERRUPT_PIN = 0x3D,
  PCI_CONFIG_MIN_GRANT = 0x3E,
  PCI_CONFIG_MAX_LATENCY = 0x3F,
};

void pci_configure();
dword pci_config_read(pci_device *device, byte offset);
void pci_config_write(pci_device *device, byte offset, dword data);
dword query_bar_size(pci_device *device, byte bar);
void pci_map_bar(pci_device *device, byte bar, dword address);
void pci_enable_bus_mastering(pci_device *device);
void pci_enable_io(pci_device *device);
void pci_enable_memory(pci_device *device);

pci_device* pci_get_device(byte bus, byte device);
pci_device* pci_find_device(word vendor_id, word device_id, byte index);

void pci_device_free(pci_device* device);



#endif

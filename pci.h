#ifndef PCI_H
#define PCI_H

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#include "output.h"

unsigned short pci_config_read(unsigned char bus, unsigned char device);
void pci_enumerate();

#endif

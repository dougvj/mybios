#ifndef PCI_H
#define PCI_H

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#include "pci.h"
#include "io.h"
#include "output.h"

unsigned long pci_config_read(unsigned char bus, unsigned char device,
                                 unsigned char func,unsigned char reg) {
    unsigned long address =  0x1 << 31 | 
                            (bus & 0x7F) << 16 | (device & 0x1F) << 11 | 
                            (func & 0x7) << 8 |  (reg & 0xFC);
//    printf("0x%X\n", address);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA); 
}

void pci_enumerate() {
    printf("Enumerating PCI Bus\n");
    for (int bus = 0; bus <= 127; bus++) {
        for (int device = 0; device < 31; device++) {
            unsigned long device_vendor = pci_config_read(bus, device, 0, 0);
            unsigned short vendor_id = device_vendor & 0xFFFF;
            unsigned short device_id = (device_vendor & 0xFFFF0000) >> 16;
            if (vendor_id != 0xFFFF) {
                printf("Found device 0x%X:0x%X at %u:%u\n", vendor_id, device_id, bus, device);
            }
        }
    }
}

#endif

#include "io.h"
#include "pci.h"
#include "output.h"
#include "post.h"
#include "postcode.h"
#include "chipset.h"

static pci_device chipset;

void chipset_init() {
    printf("Attempting to initialize Chipset i440fx\n");
    pci_device* dev;
    if (!(dev = pci_find_device(0x8086, 0x1237, 0))) {
      printf("Chipset not found\n");
      return;
    }
    chipset_shadow_rom(0xF0000, 0x10000);
    chipset = *dev;
    printf("Chipset Initialized\n");
}

bool chipset_has_rom_shadowing() {
  return true;
}

static void pam_enable_write(byte pam_register) {
  byte which_byte = pam_register & 0x3;
  dword current_config = pci_config_read(&chipset, pam_register);
  current_config |= (0x11 << (which_byte * 8));
  printf("%x, %x, curr: %x\n",(dword)which_byte, (dword)pam_register, current_config);
  pci_config_write(&chipset, pam_register, current_config);
}

static void pam_enable_read(byte pam_register) {
  byte which_byte = pam_register & 0x3;
  dword current_config = pci_config_read(&chipset, pam_register);
  current_config |= (0x22 << (which_byte * 8));
  printf("%x, %x, curr: %x\n",(dword)which_byte, (dword)pam_register, current_config);
  pci_config_write(&chipset, pam_register, current_config);
}


void chipset_shadow_rom_from_src(dword address, dword len, dword src) {
  volatile byte* region = (byte*)address;
  volatile byte* src_region = (byte*)src;
  byte regs[4];
  int num_regs = 0;
  switch(address) {
    // TODO support more regions
    case 0xC0000:
      if (len == 0x10000) {
        num_regs = 2;
        regs[0] = 0x5a;
        regs[1] = 0x5b;
      }
      break;
    case 0xF0000:
      if(len == 0x10000) {
        num_regs = 1;
        regs[0] = 0x59;
      }
  }
  if (num_regs == 0) {
    printf("Could not shadow region: address:%x, len:%x\n", address, len);
    return;
  }
  for (int i = 0; i < num_regs; i++) {
    pam_enable_write(regs[i]);
#ifdef BOCHS
    // BOCHS doesn't actually properly emulate PAM regions, we have to enable
    // DRAM read/write at the same time and then simply copy the data over.
    pam_enable_read(regs[i]);
#endif
  }
  printf("Shadowing %u bytes from %x to %x\n", len, src, address);
  for (int i = 0; i < len; i++) {
    region[i] = src_region[i];
  }
#ifndef BOCHS
  for (int i = 0; i < num_regs; i++) {
    pam_enable_read(regs[i]);
  }
#endif
}

bool chipset_has_pci() {
  return true;
}

dword chipset_pci_config_address(byte bus, byte device, byte function, byte offset) {
  return pci_type2_config_address(bus, device, function, offset);
}

dword chipset_pci_config_read(dword base_config_address, byte offset) {
  return pci_type2_config_read(base_config_address, offset);
}

void chipset_pci_config_write(dword base_config_address, byte offset, dword value) {
  pci_type2_config_write(base_config_address, offset, value);
}




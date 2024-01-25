#include "io.h"
#include "pci.h"
#include "output.h"
#include "post.h"
#include "postcode.h"
#include "chipset.h"

char CHIPSET_NAME[] = "i440fx";

static pci_device chipset;

enum pam_mode {
  PAM_DISABLE=0,
  PAM_READ_ONLY=1,
  PAM_WRITE_ONLY=2,
  PAM_READ_WRITE=3,
};

enum pam_segment {
  PAM_F0000_64=1,
  PAM_C0000_16,
  PAM_C4000_16,
  PAM_C8000_16,
  PAM_CC000_16,
  PAM_D0000_16,
  PAM_D4000_16,
  PAM_D8000_16,
  PAM_DC000_16,
  PAM_E0000_16,
  PAM_E4000_16,
  PAM_E8000_16,
  PAM_EC000_16,
};

#define PAM_BASE 0x59

static void pam_set_mode(enum pam_segment pam_segment, enum pam_mode pam_mode) {
  int byte_offset = PAM_BASE + pam_segment / 2;
  int which_nibble = pam_segment % 2;
  u8 reg = pci_config_readb(&chipset, byte_offset);
  int shift = 4 * which_nibble;
  int mask = which_nibble ? 0xCF : 0xFC;
  pci_config_writeb(&chipset, byte_offset, (reg & mask) | (pam_mode << shift));
}

static bool find_chipset() {
    pci_device* dev;
    if (!(dev = pci_find_device(0x8086, 0x1237, 0))) {
      return false;
    }
    chipset = *dev;
    return true;
}

void chipset_init() {
    printf("Attempting to initialize Chipset i440fx\n");
    if (!find_chipset()) {
      printf("Chipset not found\n");
      return;
    }
    chipset_shadow_rom(0xF0000, 0x10000);
    printf("Chipset Initialized\n");
}

void chipset_post() {
  // We have to find the chipset again because between chipset_init and chipset_post
  // the runtime memory map changes and the chipset handle is no longer valid.
  if (!find_chipset()) {
    printf("Chipset not found\n");
    return;
  }
}

bool chipset_has_rom_shadowing() {
  return true;
}

bool chipset_shadow_rom_from_src(u32 address, u32 len, u32 src) {
  volatile u8* region = (u8*)address;
  volatile u8* src_region = (u8*)src;
  enum pam_segment pam_region;
  int num_regions = 0;
  switch(address) {
    case 0xC0000:
      pam_region = PAM_C0000_16;
      goto handle_16k;
    case 0xD0000:
      pam_region = PAM_D0000_16;
      goto handle_16k;
    case 0xE0000:
      pam_region = PAM_E0000_16;
      goto handle_16k;
    handle_16k:
      if(len & 0x3FFF) {
        printf("Cannot shadow region %x with odd length %x\n", address, len);
        return false;
      }
      num_regions = len / 0x4000;
      if (num_regions > 4) {
        // This alows mapping 0xE0000 to 0x100000 in one call
        if (num_regions == 8) {
          chipset_shadow_rom_from_src(address + 0x10000, 0x10000, src + 0x10000);
        } else {
          printf("Cannot shadow region %x of length %x\n", address, len);
          return false;
        }
        num_regions = 4;
      }
      break;
    case 0xF0000:
      pam_region = PAM_F0000_64;
      if(len & 0xFFFF) {
        printf("Cannot shadow region %x with odd length %x\n", address, len);
        return false;
      }
      num_regions = 1;
      break;
    default:
      printf("Cannot shadow region %x\n", address);
      return false;
  }
  if (num_regions == 0) {
    printf("Cannot shadow region %x of length %x\n", address, len);
    return false;
  }
  for(int i = 0; i < num_regions; i++) {
    pam_set_mode(pam_region + i, PAM_WRITE_ONLY);
  }
  printf("Shadowing %u bytes from %x to %x\n", len, src, address);
  for (unsigned int i = 0; i < len; i++) {
    region[i] = src_region[i];
  }
  for(int i = 0; i < num_regions; i++) {
    pam_set_mode(pam_region + i, PAM_READ_WRITE);
  }
  return true;
}

bool chipset_has_pci() {
  return true;
}

u32 chipset_pci_config_address(u8 bus, u8 device, u8 function, u8 offset) {
  return pci_type2_config_address(bus, device, function, offset);
}

u32 chipset_pci_config_read(u32 base_config_address, u8 offset) {
  return pci_type2_config_read(base_config_address, offset);
}

void chipset_pci_config_write(u32 base_config_address, u8 offset, u32 value) {
  pci_type2_config_write(base_config_address, offset, value);
}




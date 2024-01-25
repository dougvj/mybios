#include "chipset.h"

// TODO I don't think this works
void reset() {
  asm("mov %cr0, %eax");
  asm("and $0xFFFFFFFE, %eax");
  asm("mov %eax, %cr0");
  asm("jmp $0xF000, $0xFFF0");
  //asm("xor %%eax, %%eax; lidt %0; int3" :: "m" (null_idtr));
}

bool chipset_shadow_rom(u32 address, u32 size) {
  return chipset_shadow_rom_from_src(address, size, address);
}

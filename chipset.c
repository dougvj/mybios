#include "chipset.h"

void reset() {
  asm("mov %cr0, %eax");
  asm("and $0xFFFFFFFE, %eax");
  asm("mov %eax, %cr0");
  asm("jmp $0xF000, $0xFFF0");
  //asm("xor %%eax, %%eax; lidt %0; int3" :: "m" (null_idtr));
}

void chipset_shadow_rom(dword address, dword size) {
#ifdef BOCHS
  return;
#endif
  chipset_shadow_rom_from_src(address, size, address);
}

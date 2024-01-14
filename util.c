#include "chipset.h"
#include "util.h"

#define call_real_mode_asm() \
      "mov %7, %%bx;" \
      "mov %8, %%cx;" \
      "mov %9, %%dx;" \
      "mov %10, %%si;" \
      "mov %11, %%di;" \
      "push %%eax;" \
      "pushw %12;" \
      "pushw %13;" \
      "mov %6, %%ax;" \
      "call call_real_mode;" \
      "add $4, %%esp;" \
      "push %%ax;" \
      "mov 2(%%esp), %%eax;" \
      "push %%bx;" \
      "push %%dx;" \
      "push %%cx;" \
      "push %%si;" \
      "push %%di;" \
      "mov 10(%%esp), %%bx;" \
      "mov %%bx, %0;" \
      "mov 8(%%esp), %%bx;" \
      "mov %%bx, %1;" \
      "mov 6(%%esp), %%bx;" \
      "mov %%bx, %2;" \
      "mov 4(%%esp), %%bx;" \
      "mov %%bx, %3;" \
      "mov 2(%%esp), %%bx;" \
      "mov %%bx, %4;" \
      "mov (%%esp), %%bx;" \
      "mov %%bx, %5;" \
      "add $16, %%esp;" \

void real_mode_call(real_mode_call_params* params)
{
  asm(
      call_real_mode_asm()
      :
      "=m"(params->ax), "=m"(params->bx), "=m"(params->cx), "=m"(params->dx),
      "=m"(params->si), "=m"(params->di)
      :
      "m"(params->ax), "m"(params->bx), "m"(params->cx), "m"(params->dx),
      "m"(params->si), "m"(params->di),
      "m"(params->segment), "m"(params->offset)
      : "ebx", "ecx", "edx", "esi", "edi"
      );
}

extern void call_int();

// DIfference is that we use di in the real_mode_int for the interrupt vector.
//
// maybe should use a stack param?
void real_mode_int(real_mode_int_params* params) {
  word segment = 0xF000;
  // Found in interrupt.asm
  word offset = (dword)(call_int) & 0xFFFF;
  asm(
      call_real_mode_asm()
      :
      "=m"(params->ax), "=m"(params->bx), "=m"(params->cx), "=m"(params->dx),
      "=m"(params->si), "=m"(params->interrupt)
      :
      "m"(params->ax), "m"(params->bx), "m"(params->cx), "m"(params->dx),
      "m"(params->si), "m"(params->interrupt),
      "m"(segment), "m"(offset)
      : "ebx", "ecx", "edx", "esi", "edi"
      );
}

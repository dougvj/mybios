#include "vga.h"
#include "output.h"
#include "runtime_init.h"
#include "postcode.h"
#include "post.h"
#include "chipset.h"
#include "ata.h"

void main() {
    initRuntime();
    vgaCls();
    printf("MyBIOS v0.01 Alpha. Doug Johnson (2018). C Code Entry Point\n");
    asm("xchg %bx, %bx");
    initChipset();
    doPost();
    ataInit();
    asm("hlt");
}


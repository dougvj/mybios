#include "vga.h"
#include "output.h"
#include "runtime_init.h"
#include "postcode.h"
#include "post.h"
#include "chipset.h"
#include "ata.h"
#include "pci.h"

char test_data_init[] = "Runtime initialized successfully\n";

void main() {
    vgaCls();
    initRuntime();
    printf("MyBIOS %s. Doug Johnson (%s). C Code Entry Point\n", "Pre-Alpha", "2020");
    printf("%s", test_data_init);
    initChipset();
    doPost();
    pci_enumerate();
    ataInit();
    asm("hlt");
}


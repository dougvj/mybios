#include "vga.h"
#include "output.h"
#include "runtime_init.h"
#include "postcode.h"
#include "post.h"
#include "chipset.h"
#include "ata.h"
#include "pci.h"

char test_data_init[] = "Runtime initialized successfully\n";

void find_boundary_signatures() {
    printf("Searching for other banks' signatures 0x%X\n", 0xDEADBEEF);
    int count = 0;
    unsigned int start = 0x0;
    do {
        if (*(unsigned int*)start == 0xDEADBEEF) {
            printf("Found boundary at 0x%X\n", start);
            count++;
        } 
        start += 0x10000;
    } while (start != 0x0);
    printf("Found %d boundaries.\n", count);
}

void main() {
    postCode(0x11);
    vgaCls();
    postCode(0x12);
    initRuntime();
    postCode(0x13);
    printf("MyBIOS %s. Doug Johnson (%s). C Code Entry Point\n", "Pre-Alpha", "2020");
    printf("%s", test_data_init);
    initChipset();
    doPost();
    pci_enumerate();
    ataInit();
    find_boundary_signatures();
    asm("hlt");
}


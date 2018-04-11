#include "output.h"
#include "postcode.h"

void printRamMsg(unsigned int ram, const char* msg) {
    printf("%d KB RAM %s\r", ram/1024, msg);
}


unsigned int probeRam(unsigned int start, unsigned int stride, int print_message) {
    unsigned int end = 0xffffffff;
    unsigned int bytes_found = 0;
    unsigned int i = 0;
    for (i = start + stride - 4; i < end; i += stride) {
        volatile int* test_point = (int*) i;
        int old_val = *test_point;
        *test_point = 0xAA55AA55;
        asm("wbinvd");
        if (*test_point == 0xAA55AA55) {
            *test_point = 0x55AA55AA;
            asm("wbinvd");
            if (*test_point == 0x55AA55AA) {
                bytes_found = (i + 4 - start);
                if (print_message && (bytes_found / stride) % 64 == 0 && bytes_found > 0) {
                    printRamMsg(bytes_found, "FOUND");
                }
            } else {
                break;
            }
        }
        else {
            break;
        }
        *test_point = old_val;
    }
    if (print_message && bytes_found > 0) {
        printRamMsg(bytes_found, "FOUND");
    }
    return bytes_found;
}

unsigned int testRam(unsigned int start, unsigned int end, int print_message) {
    unsigned int bytes = 0;
    unsigned int i = 0;
    for (i = start; i < start + end; i += 4) {
        volatile int* test_point = (int*) i;
        int old_val = *test_point;
        *test_point = 0xAA55AA55;
        asm("wbinvd");
        if (*test_point == 0xAA55AA55) {
            *test_point = 0x55AA55AA;
            asm("wbinvd");
            if (*test_point == 0x55AA55AA) {
                bytes += 4;
                if (print_message && bytes % 1024 == 0) {
                    printRamMsg(bytes, "OK");
                }
            } else {
                printRamMsg(bytes, "Failed");
                break;
            }
        }
        else {
            printRamMsg(bytes, "Failed");
            break;
        }
        *test_point = old_val;
    }
    if (print_message) {
        printRamMsg(bytes, "OK");
    }
    return bytes;
}

unsigned int probeAll(unsigned int stride, unsigned int top, int print_message) {
    unsigned int total = 0;
    for (unsigned int i = 0; i < top; ) {
        postCode(0x2);
        unsigned int ram = probeRam(i, 1024, 1);
        postCode(0x3);
        if (ram > 0) {
            printf("\nFound RAM region of size %d KB From 0x%x to 0x%x\n", ram / 1024, i, i + ram - 1);
            i += (ram - (ram % stride));
        }
        else {
            i += stride;
        }
        total += ram;
    }
    return total;
}

void checkRam() {
    unsigned int lower = probeRam(0x0, 0x400, 1);
    printf("\n");
    unsigned int upper = probeRam(0x100000, 0x400, 1);
    printf("\n%d KB Conventional Memory\n", lower / 1024);
    printf("%d KB Upper Memory\n", upper / 1024);
    printf("%d KB Total\n", (lower + upper) / 1024);
    printf("Testing Conventional Memory\n");
    testRam(0x0, lower, 1);
    printf("\nTesting Upper Memory\n");
    testRam(0x100000, upper, 1);


}

void doPost() {
    checkRam();
}

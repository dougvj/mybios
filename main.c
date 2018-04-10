#include "vga.h"
#include "output.h"
#include "runtime_init.h"

void print() {
    static int count = 0;
    char* loc = (char*)(0xB8200);
    loc[count * 2] = count + 0x30;
    loc[count * 2 + 1] = 0x1f;
    count++;
}

void main() {
    initRuntime();
    vgaCls();
    char* loc = (char*)0xB8140;
    char* msg = "And now we are executing C!";
    char* w;
    for (int i = 0; msg[i] != '\0'; i++) {
        int off = i * 2;
        loc[off] = msg[i];
        loc[off + 1] = 0x1f;
    }
    print();
    print();
    print();
    print();
    for (;;) {
        asm("xchg %bx, %bx");
        printf("%s: %i 0x%x\n", "Now does this work or nah?", loc, loc);
    }
}


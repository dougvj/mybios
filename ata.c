#include "output.h"
#include "io.h"
#include "postcode.h"
#define PRIMARY 0x1F0
#define SECONDARY 0x170
static int _base_port = 0;

unsigned short ataReadData() {
    return inw(_base_port + 0);
}

void ataWriteData(unsigned short data) {
    outw(_base_port + 0, data);
}

void ataSectorCount(unsigned short count) {
    outb(_base_port + 2, count);
}

void ataLBA(unsigned int addr) {
    outb(_base_port + 3, addr & 0xFF);
    outb(_base_port + 4, (addr >> 8) & 0xFF);
    outb(_base_port + 5, (addr >> 16) & 0xFF);
}

int ataDetectController() {
    asm("xchg %bx, %bx");
    outb(_base_port + 2, 0xA5);
    unsigned char controller = inb(_base_port + 2);
    return (controller == 0xA5);
}

void ataDrive(unsigned char drv) {
    outb(_base_port + 6, drv);
}

void ataCommand(unsigned char cmd) {
    outb(_base_port + 7, cmd);
}

unsigned char ataStatus() {
    return inb(_base_port + 7);
}

#define ATA_MASTER   0xA0
#define ATA_SLAVE    0xB0
#define ATA_IDENTIFY 0xEC
void ataIdentify(unsigned char drv) {
    ataDrive(drv);
    ataDrive(drv);
    ataDrive(drv);
    ataDrive(drv);
    ataSectorCount(0);
    ataSectorCount(0);
    ataSectorCount(0);
    ataSectorCount(0);
    ataSectorCount(0);
    ataLBA(0);
    ataLBA(0);
    ataLBA(0);
    ataLBA(0);
    ataLBA(0);
    ataLBA(0);
    ataCommand(ATA_IDENTIFY);
    ataCommand(ATA_IDENTIFY);
    ataCommand(ATA_IDENTIFY);
    ataCommand(ATA_IDENTIFY);
    ataCommand(ATA_IDENTIFY);
    ataCommand(ATA_IDENTIFY);
    ataCommand(ATA_IDENTIFY);
    unsigned char status;
    while (status = ataStatus()) {
        if (!(status & 0x80)) {
            break;
        }
        postCode(status);
    }
    if (status == 0) {
        printf(".Not Found\n");
        return;
    }
    printf(".");
    while (status = ataStatus()) {
        if (status & 0x8) {
            break;
        }
        if (status & 0x1) {
            printf(".Error\n");
            return;
        }
        postCode(status);
    }
    unsigned short ident[256];
    for (int i = 0; i < 256; i++) {
        ident[i] = ataReadData();
    }
    char model[41];
    for (int i = 27; i < 47; i++) {
        model[((i - 27) << 1) + 1] = ident[i] & 0xFF;
        model[((i - 27) << 1)] = (ident[i] & 0xFF00) >> 8;
    }
    model[40] = '\0';
    printf("%s\n", model);
}

void ataSetController(int controller) {
    _base_port = controller;
}

void ataInit() {
    ataSetController(PRIMARY);
    if (!ataDetectController()) {
        printf("Primary IDE Controller not found\n");
    } else {
        printf("Scanning Primary Master...");
        ataIdentify(ATA_MASTER);
        printf("Scanning Primary Slave...");
        ataIdentify(ATA_SLAVE);
    }
    ataSetController(SECONDARY);
    if (!ataDetectController()) {
        printf("Secondary IDE Controller not found\n");
    } else {
        printf("Scanning Secondary Master...");
        ataIdentify(ATA_MASTER);
        printf("Scanning Secondary Slave...");
        ataIdentify(ATA_SLAVE);
    }
}

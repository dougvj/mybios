#include "output.h"
#include "io.h"
#include "postcode.h"
#include "interrupts.h"
#define PRIMARY 0x1F0
#define SECONDARY 0x170
#define ATA_MASTER   0xA0
#define ATA_SLAVE    0xB0
#define ATA_IDENTIFY 0xEC
#define ATA_READ_SECTORS 0x20
static int _base_port = 0;

typedef struct drive_info {
    char model[41];
    unsigned int blocks;
    unsigned int controller;
    bool lba;
    bool slave;
} drive_info_t;

int num_drives;
drive_info_t drives[4];

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
    outb(_base_port + 2, 0xA5);
    unsigned char controller = inb(_base_port + 2);
    return (controller == 0xA5);
}

void ataDriveHead(unsigned char drv) {
    outb(_base_port + 6, drv);
}

void ataCommand(unsigned char cmd) {
    outb(_base_port + 7, cmd);
}

unsigned char ataStatus() {
    return inb(_base_port + 7);
}

void ataIdentify(unsigned char drv) {
    ataDriveHead(drv);
    ataSectorCount(0);
    ataLBA(0);
    ataCommand(ATA_IDENTIFY);
    unsigned char status;
    while ((status = ataStatus())) {
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
    while ((status = ataStatus())) {
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
    unsigned int blocks = ident[60] | (ident[61] << 16);
    bool lba = blocks > 0;
    printf("Detected\n  %s ", model);
    if (blocks > 0) {
        unsigned int size_mb = (blocks * 512) / 1024 / 1024;
        printf("LBA, %d MB\n", size_mb);
    } else {
        if (ident[53] & 0x1) {
            blocks = ident[57] | (ident[58] << 16);
            unsigned int size_mb = (blocks * 512) / 1024 / 1024;
            printf("CHS, %d MB\n", size_mb);
        } else {
            printf("??? Older drive suspected\n");
        }
    }
    drives[num_drives].controller = _base_port;
    drives[num_drives].lba = lba;
    drives[num_drives].slave = drv == ATA_SLAVE;
    drives[num_drives].blocks = blocks;
    for (int i = 0; i < 40; i++) {
        drives[num_drives].model[i] = model[i];
    }
    num_drives++;
}

void ataSetController(int controller) {
    _base_port = controller;
}

void interrupt ataHandler14(interrupt_frame_t* frame) {
    //printf("ATA Primary Interrupt at %x\n", frame->ip);
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void interrupt ataHandler15(interrupt_frame_t* frame) {
    //printf("ATA Secondary Interrupt at %x\n", frame->ip);
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void ataInit() {
    interrupts_set_interrupt_handler(IRQ14, ataHandler14);
    interrupts_set_interrupt_handler(IRQ15, ataHandler15);
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

int numDrives() {
  return num_drives;
}

int ataRead(int drive, int sector, int count, char* buffer) {
  if (drive >= num_drives || drive < 0) {
    printf("Drive %d not found\n", drive);
    return -1;
  }
  drive_info_t* info = &drives[drive];
  ataSetController(info->controller);
  if (info->lba) {
    ataDriveHead((0xE0 | info->slave << 6) | (sector >> 24));
    outb(_base_port + 1, 0);
    ataSectorCount(count);
    ataLBA(sector);
    ataCommand(ATA_READ_SECTORS);
    unsigned char status;
    int c = 0;
    while (c< 512 * count) {
      while ((status = ataStatus())) {
        if (!(status & 0x80)) {
          break;
        }
      }
      for (int i = 0; i < 256; i++) {
        unsigned short data = ataReadData();
        buffer[c++] = data & 0xFF;
        buffer[c++] = (data & 0xFF00) >> 8;
      }
    }
    printf("Read %d bytes\n", c);
    return c;
  } else {
    printf("CHS not supported\n");
    return -1;
  }
}

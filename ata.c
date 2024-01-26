#include "ata.h"
#include "interrupt.h"
#include "io.h"
#include "output.h"
#include "postcode.h"

#define ATA_IDENTIFY 0xEC
#define ATA_READ_SECTORS 0x20

enum ata_regs {
  ATA_DATA = 0,
  ATA_ERROR = 1,
  ATA_SECTOR_COUNT = 2,
  ATA_LBA_LO = 3,
  ATA_LBA_MID = 4,
  ATA_LBA_HI = 5,
  ATA_DRIVE_HEAD = 6,
  ATA_STATUS = 7,
  ATA_COMMAND = 7
};

enum ata_status {
  ATA_STATUS_ERR = 0x01,
  ATA_STATUS_DRQ = 0x08,
  ATA_STATUS_SRV = 0x10,
  ATA_STATUS_DF = 0x20,
  ATA_STATUS_RDY = 0x40,
  ATA_STATUS_BSY = 0x80
};

enum ata_commands {
  ATA_CMD_READ_PIO = 0x20,
  ATA_CMD_READ_PIO_EXT = 0x24,
  ATA_CMD_READ_DMA = 0xC8,
  ATA_CMD_READ_DMA_EXT = 0x25,
  ATA_CMD_WRITE_PIO = 0x30,
  ATA_CMD_WRITE_PIO_EXT = 0x34,
  ATA_CMD_WRITE_DMA = 0xCA,
  ATA_CMD_WRITE_DMA_EXT = 0x35,
  ATA_CMD_CACHE_FLUSH = 0xE7,
  ATA_CMD_CACHE_FLUSH_EXT = 0xEA,
  ATA_CMD_PACKET = 0xA0,
  ATA_CMD_IDENTIFY_PACKET = 0xA1,
  ATA_CMD_IDENTIFY = 0xEC
};

// divide by 2 because we lookup u16s
enum ata_identify {
  ATA_IDENT_DEVICETYPE = 0,
  ATA_IDENT_CYLINDERS = 1,
  ATA_IDENT_HEADS = 3,
  ATA_IDENT_SECTORS = 6,
  ATA_IDENT_SERIAL = 10,
  ATA_IDENT_MODEL = 27,
  ATA_IDENT_FIELDVALID = 53,
  ATA_IDENT_MAX_LBA = 60,
  ATA_IDENT_COMMANDSETS = 82,
  ATA_IDENT_MAX_LBA_EXT = 100
};

struct dev_ata {
  u32 base;
  u32 irq;
  bool slave_selected;
};


static bool detect_controller(dev_ata *dev) {
  outb(dev->base + 2, 0xA5);
  unsigned char controller = inb(dev->base + 2);
  return (controller == 0xA5);
}

static u8 read_drive_head(dev_ata *dev) { return inb(dev->base + ATA_DRIVE_HEAD); }


static u8 status(dev_ata *dev) { return inb(dev->base + ATA_STATUS); }

bool ata_identify(dev_ata *dev, bool slave, ata_drive *info) {
  info->dev = NULL;
  ata_drive_select(dev,slave);
  outb(dev->base + 0x206, 0x2);
  outb(dev->base + ATA_SECTOR_COUNT, 0);
  outb(dev->base + ATA_LBA_LO, 0);
  outb(dev->base + ATA_LBA_MID, 0);
  outb(dev->base + ATA_LBA_HI, 0);
  outb(dev->base + ATA_COMMAND, ATA_IDENTIFY);
  u8 s;
  while ((s = status(dev))) {
    if (!(s & ATA_STATUS_BSY)) {
      break;
    }
  }
  if (s == 0) {
    printf(".Not Found\n");
    return false;
  }
  printf(".");
  while ((s = status(dev))) {
    if (s & ATA_STATUS_DRQ) {
      break;
    }
    if (s & ATA_STATUS_ERR) {
      printf(".Error\n");
      return false;
    }
  }
  u16 ident[256];
  for (int i = 0; i < 256; i++) {
    ident[i] = inw(dev->base + ATA_DATA);
  }
  int j = 0;
  for (int i = ATA_IDENT_MODEL; i < ATA_IDENT_MODEL + 20; i++) {
    info->model[j++] = (ident[i] & 0xFF00) >> 8;
    info->model[j++] = ident[i] & 0xFF;
  }
  info->model[40] = '\0';
  u32 blocks = ident[60] | (ident[61] << 16);
  u64 blocks48 = ident[100] | (ident[101] << 16) | ((u64)ident[102] << 32) |
                 ((u64)ident[103] << 48);
  bool lba = blocks > 0;
  u16 heads = ident[ATA_IDENT_HEADS];
  u16 sectors = ident[ATA_IDENT_SECTORS];
  u16 cylinders = ident[ATA_IDENT_CYLINDERS];

  printf("Detected\n  %s ", info->model);
  if (blocks > 0) {
    unsigned int size_mb = (blocks * 512) / 1024 / 1024;
    printf("LBA, %d MB", size_mb);
    // Setup translation
    // TODO set this by drive size
    /*heads = 128;
    sectors = 63;
    cylinders = size_mb / heads / sectors;*/
  } else {
    if (ident[53] & 0x1) {
      blocks = ident[57] | (ident[58] << 16);
      unsigned int size_mb = (blocks * 512) / 1024 / 1024;
      printf("CHS, %d MB", size_mb);
    } else {
      blocks = heads * sectors * cylinders;
      unsigned int size_mb = (blocks * 512) / 1024 / 1024;
      printf("CHS(?):, %d MB", size_mb);
    }
    printf("  %dc %dh %ds\n", cylinders, heads, sectors);
  }
  info->flags = 0;
  info->flags |= lba ? ATA_DRIVE_LBA28 : 0;
  info->flags |= blocks48 > 0 ? ATA_DRIVE_LBA48 : 0;
  info->flags |=  slave ? ATA_DRIVE_SLAVE : 0;
  info->type = ident[ATA_IDENT_DEVICETYPE];
  info->cylinders = cylinders;
  info->heads = heads;
  info->sectors = sectors;
  info->blocks28 = blocks;
  info->blocks48 = blocks48;
  info->dev = dev;
  return true;
}

void ata_wait(dev_ata *dev) {
  u8 s;
  while ((s = status(dev))) {
    if (!(s & ATA_STATUS_BSY)) {
      break;
    }
  }
}

void ata_drive_select(dev_ata *dev, bool slave) {
  if (dev->slave_selected == slave) {
    return;
  }
  outb(dev->base + ATA_DRIVE_HEAD, 0xA0 | (slave ? 0x10 : 0));
  u8 s;
  int count = 0;
  while (1) {
    s = status(dev);
    count++;
    if (!(s & ATA_STATUS_BSY)) {
      if (count > 15) {
        break;
      }
    }
  }
  dev->slave_selected = slave;
}


static void irq_handler(enum itr_irq irq, void* data) {
  dev_ata unused *dev = (dev_ata *)data;
  //printf("ATA Interrupt for %x\n", dev->base);
}


static dev_ata ata_devices[4];
static int num_devices = 0;

dev_ata* ata_init(u32 base, u32 irq) {
  if (num_devices >= 4) {
    printf("Too many ATA devices\n");
    return NULL;
  }
  dev_ata *dev = &ata_devices[num_devices++];
  dev->base = base;
  dev->irq = irq;
  if (!detect_controller(dev)) {
    num_devices--;
    return NULL;
  }
  dev->slave_selected = true;
  ata_drive_select(dev, false);
  itr_set_irq_handler(irq, irq_handler, dev);
  return dev;
}

u32 ata_read_lba(dev_ata *dev, u32 sector, u16 count, char *buffer) {
  outb(dev->base + ATA_DRIVE_HEAD, (0xE0 | (dev->slave_selected ? 0x10 : 0)) | ((sector >> 24) & 0xF));
  outb(dev->base + ATA_ERROR, 0);
  outb(dev->base + ATA_SECTOR_COUNT, count);
  outb(dev->base + ATA_LBA_LO, sector);
  outb(dev->base + ATA_LBA_MID, sector >> 8);
  outb(dev->base + ATA_LBA_HI, sector >> 16);
  outb(dev->base + ATA_COMMAND, ATA_CMD_READ_PIO);
  u8 s;
  int c = 0;
  while (c < 512 * count) {
    while ((s = status(dev))) {
      if (!(s & 0x80)) {
        break;
      }
    }
    for (int i = 0; i < 256; i++) {
      u16 data = inw(dev->base + ATA_DATA);
      buffer[c++] = data & 0xFF;
      buffer[c++] = (data & 0xFF00) >> 8;
    }
  }
  printf("Read %d bytes\n", c);
  return c;
}


u32 ata_read_chs(dev_ata* dev, u32 cylinder, u8 head, u16 sector, u16 count, char* buffer) {
  outb(dev->base + ATA_DRIVE_HEAD, (0xA0 | (dev->slave_selected ? 0x10 : 0)) | head);
  outb(dev->base + ATA_ERROR, 0);
  outb(dev->base + ATA_SECTOR_COUNT, count);
  outb(dev->base + ATA_LBA_LO, sector);
  outb(dev->base + ATA_LBA_MID, cylinder & 0xFF);
  outb(dev->base + ATA_LBA_HI, (cylinder >> 8) & 0xFF);
  outb(dev->base + ATA_COMMAND, ATA_CMD_READ_PIO);
  u8 s;
  int c = 0;
  while (c < 512 * count) {
    while ((s = status(dev))) {
      if (!(s & 0x80)) {
        break;
      }
    }
    for (int i = 0; i < 256; i++) {
      u16 data = inw(dev->base + ATA_DATA);
      buffer[c++] = data & 0xFF;
      buffer[c++] = (data & 0xFF00) >> 8;
    }
  }
  printf("Read %d bytes\n", c);
  return c;
}




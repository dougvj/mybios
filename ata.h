#ifndef ATA_H
#define ATA_H
#include "types.h"
#include "interrupt.h"

enum ata_drive_flags {
  ATA_DRIVE_LBA28 = 0x200,
  ATA_DRIVE_LBA48 = 0x400,
  ATA_DRIVE_SLAVE = 0x800
};

typedef struct dev_ata dev_ata;

typedef struct ata_drive {
  u16 type;
  char model[41];
  u64 blocks48;
  u32 blocks28;
  u16 cylinders;
  u16 heads;
  u16 sectors;
  enum ata_drive_flags flags;
  dev_ata* dev;
} ata_drive;

enum ata_default_base {
  ATA_BASE_PRIMARY = 0x1F0,
  ATA_BASE_SECONDARY = 0x170
};

enum ata_default_irq {
  ATA_IRQ_PRIMARY = IRQ14,
  ATA_IRQ_SECONDARY = IRQ15
};

dev_ata *ata_init(u32 base, u32 irq);
bool ata_identify(dev_ata *dev, bool slave, ata_drive *info);
u32 ata_read_lba(dev_ata *dev, u32 sector, u16 count, char *buffer);
u32 ata_write_lba(dev_ata* dev, u32 sector, u16 count, char *buffer);
u32 ata_read_lba48(dev_ata* dev, u64 sector, u16 count, char *buffer);
u32 ata_write_lba48(dev_ata* dev, u64 sector, u16 count, char *buffer);
u32 ata_read_chs(dev_ata* dev, u32 cylinder, u8 head, u16 sector, u16 count,
                 char *buffer);
u32 ata_write_chs(dev_ata* dev, u32 cylinder, u8 head, u16 sector, u16 count,
                  char *buffer);
bool ata_drive_reset(dev_ata *dev);
void ata_drive_select(dev_ata *dev, bool slave);
// Automatically selects the drive and the read/write function
u32 ata_drive_read(ata_drive *drive, u64 sector, u16 count, char *buffer);
u32 ata_drive_write(ata_drive *drive, u64 sector, u16 count, char *buffer);
#endif

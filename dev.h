// Device enumeration and management
#ifndef __DEV_H__
#define __DEV_H__
// Device enumeration and management
#include "ata.h"
#include "timer.h"

extern dev_timer* dev_timer_primary;
extern ata_drive dev_ata_drives[4];
// TODO add the rest when we get to converting them to a handle based system


#endif

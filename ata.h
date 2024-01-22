#ifndef ATA_H
#define ATA_H


void ataInit();
int numDrives();
int ataRead(int drive, int sector, int count, char* buffer);
#endif

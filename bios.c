#include "bios.h"
#include "interrupt.h"
#include "io.h"
#include "output.h"
#include "keyboard.h"
#include "ata.h"
#include "dev.h"
#include "bda.h"


void bios_handle_disk_interrupt(u8 vector, itr_frame_real_mode* frame, void* unused data) {
      printf("Got disk interrupt: %x\n", vector);
      switch(frame->ax >> 8) {
        case 0x8: {
          int drive = frame->dl;
          printf("Got get disk parameters for %x \n", drive);
          if (drive == 0x80) {
            ata_drive* info = &dev_ata_drives[0];
            u8 sectors_per_track = info->sectors;
            u8 heads = info->heads;
            u32 cylinders = info->cylinders;
            printf("Cylinders: %d\n", cylinders);
            printf("Heads: %d\n", heads);
            printf("Sectors per track: %d\n", sectors_per_track);
            frame->ah = 0;
            frame->ch = cylinders & 0xFF;
            frame->cl = sectors_per_track | ((cylinders >> 8) & 0x3) << 6;
            frame->dh = heads;
            frame->dl = 1;
            printf("ax: %x\n", frame->ax);
            printf("cx: %x\n", frame->cx);
            printf("dx: %x\n", frame->dx);
            frame->flags &= 0xFFFE;
          } else {
            printf("Drive %x not found\n", drive);
            frame->flags |= 0x1;
            frame->al = 0x01;
          }
        } break;
        case 0x2: {
          int drive = frame->dl;
          if (drive == 0x80) {
            ata_drive* info = &dev_ata_drives[0];
            /*printf("Got read disk sectors\n");
            printf("ax: %x\n", frame->ax);
            printf("cx: %x\n", frame->cx);
            printf("dx: %x\n", frame->dx);*/
            int num_sectors = frame->al;
            int cylinder = frame->ch | ((u32)(frame->cl & 0xC0) << 2);
            int head = frame->dh;
            int sector = frame->cl & 0x3F;
            printf("Reading %d sectors from drive %d, cylinder %d, head %d, sector %d\n", num_sectors, drive, cylinder, head, sector);
            char* dest = (char*)((u32)(frame->es <<4) + frame->bx);
            printf("Dest: %x\n", dest);
            printf("ss:sp: %x:%x\n", frame->ss, frame->esp);
            if (info->flags & ATA_DRIVE_LBA28) {
              u32 lba = (cylinder * (info->heads + 1) + head) * info->sectors + sector - 1;
              ata_read_lba(info->dev, lba, num_sectors, dest);
            } else{
              ata_read_chs(info->dev, cylinder, head, sector, num_sectors, dest);
            }
            // clear carry flag
            frame->flags &= 0xFFFE;
            frame->al =  num_sectors;
            frame->ah = 0x00;
          }
        } break;
        case 0x3: {
          int drive = frame->dl;
          if (drive == 0x80) {
            ata_drive* info = &dev_ata_drives[0];
            /*printf("Got read disk sectors\n");
            printf("ax: %x\n", frame->ax);
            printf("cx: %x\n", frame->cx);
            printf("dx: %x\n", frame->dx);*/
            int num_sectors = frame->al;
            int cylinder = frame->ch | ((u32)(frame->cl & 0xC0) << 2);
            int head = frame->dh;
            int sector = frame->cl & 0x3F;
            printf("Writing %d sectors from drive %d, cylinder %d, head %d, sector %d\n", num_sectors, drive, cylinder, head, sector);
            char* src = (char*)((u32)(frame->es <<4) + frame->bx);
            if (info->flags & ATA_DRIVE_LBA28) {
              u32 lba = (cylinder * (info->heads + 1) + head) * info->sectors + sector - 1;
              ata_write_lba(info->dev, lba, num_sectors, src);
            } else{
              ata_write_chs(info->dev, cylinder, head, sector, num_sectors, src);
            }
            // clear carry flag
            frame->flags &= 0xFFFE;
            frame->al =  num_sectors;
            frame->ah = 0x00;
          }
        } break;
        case 0x0:
          printf("Got reset disk system\n");
          // Just assume it worked
          frame->flags &= 0xFFFE;
          frame->al = 0x00;
          break;
        default:
          printf("Got unknown disk interrupt: %x\n", frame->ax >> 8);
          frame->al = 0x01;
          frame->flags |= 0x1;
          break;
      }
}

void bios_handle_keyboard_interrupt(u8 unused vector, itr_frame_real_mode* frame, void* unused data) {
      //printf("Got keyboard interrupt: %x\n", vector);
      switch(frame->ah) {
        case 0x1:
        case 0x11:{
          u8 scancode = keyboard_get_scancode();
          if (scancode > 0) {
            frame->flags &= ~0x40;
            frame->ah = scancode;
            u8 ascii = keyboard_map(scancode);
            frame->al = ascii;
            printf("Got keyboard check keystroke: %x %x\n", scancode, ascii);
          } else {
            frame->flags |= 0x40;
            frame->al = 0;
          }
        } break;
        case 0x0:
        case 0x10:
          printf("Got keyboard read\n");
          while (!keyboard_available());
          u8 scancode = keyboard_pop_scancode();
          frame->ah = scancode;
          u8 ascii = keyboard_map(scancode);
          frame->al = ascii;
          printf("Got keyboard read_keystroke: %x %x\n", scancode, ascii);
          break;
        case 0x2: // shift status
          //SET_L(frame->ax, 0x10);
          frame->ax = bda.keyboard_flags_xt;
          break;
        case 0x12:
          frame->ax = bda.keyboard_flags_at;
          break;
        default:
          printf("Unhandled KB %x\n", frame->al);
          break;
      }
}

void bios_handle_memory_size_interrupt(u8 vector, itr_frame_real_mode* frame, void* unused data) {
  printf("Got memory size interrupt: %x\n", vector);
  frame->ax= 640-16;
}

void bios_handle_get_system_time(u8 unused vector, itr_frame_real_mode* frame, void* unused data) {
  printf("Got system time interrupt: %x, AH:0x%x\n", vector, frame->ah);
  u32 ticks = bda.irq0_counter;
  u32 seconds = ticks / 18;
  frame->dx = seconds & 0xFFFF;
  frame->cx = (seconds >> 16) & 0xFFFF;
  printf("Seconds: %d\n", seconds);

  frame->flags &= 0xFFFE;
}

void bios_handle_get_equipment_list(u8 vector, itr_frame_real_mode* frame, void* unused data) {
  printf("Got equipment list interrupt: %x\n", vector);
  int num_floppies = 0;
  int math_coprocessor = 1;
  int initial_video_mode = 0x10;
  int num_serial_ports = 2;
  int game_port = 0;
  int num_parallel_ports = 1;
  frame->ax = (num_floppies ? 1 : 0) |
      (math_coprocessor << 1) | (initial_video_mode << 4) |
      (num_serial_ports << 9) | (game_port << 12) | (num_parallel_ports << 14);
}

void bios_handle_serial_port(u8 vector, itr_frame_real_mode* frame, void* unused  data) {
  printf("Got serial port interrupt: %x\n", vector);
  switch (frame->al) {
    case 0x0:
      printf("Got serial port reset\n");
      frame->flags &= 0xFFFE;
      break;
    case 0x1:
      printf("Got serial port initialize\n");
      frame->flags &= 0xFFFE;
      break;
    case 0x2:
      printf("Got serial port send\n");
      frame->flags &= 0xFFFE;
      break;
    case 0x3:
      printf("Got serial port receive\n");
      frame->flags &= 0xFFFE;
      break;
    case 0x4:
      printf("Got serial port status\n");
      frame->flags &= 0xFFFE;
      break;
    default:
      printf("Got unknown serial port interrupt: 0x%x\n", frame->ax);
      frame->flags |= 0x1;
      break;
  }
}

void bios_handle_int15(u8 vector, itr_frame_real_mode* frame, void* unused data) {
  //printf("Got int15 interrupt: %x\n", vector);
  switch (frame->ah) {
    case 0x87:
      printf("Got int16 extended memory copy\n");
      u8* gdt = (u8*)((frame->es << 4) + frame->si);
      u16* src = (u16*)((u32)gdt[0x12] | (u32)gdt[0x13] << 8 | (u32)gdt[0x14] << 16);
      u16* dest = (u16*)((u32)gdt[0x1A] | (u32)gdt[0x1B] << 8 | (u32)gdt[0x1C] << 16);
      printf("Copying %d words from %x to %x\n", frame->cx, src, dest);
      for (int i = 0; i < frame->cx; i++) {
        dest[i] = src[i];
      }
      frame->flags &= 0xFFFE;
      frame->ah = 0;
      break;
    case 0x88:
      printf("Got int15 get extended memory size\n");
      frame->flags &= 0xFFFE;
      printf("Memory size: %d\n", bda.memory_kb);
      frame->al =  bda.memory_kb - 1024;
      printf("Extended memory size: %d\n", bda.memory_kb - 1024);
      break;
    case 0x24:
      switch(frame->al) {
        case 0x0:
          // Disable A20
          printf("Got int15 disable A20\n");
          frame->flags &= 0xFFFE;
          break;
        case 0x3:
          // Query support
          printf("Got int15 query A20\n");
          frame->flags &= 0xFFFE;
          frame->bx = 0;
      }
      break;
    case 0xC0:
      printf("Got int15 get system configuration\n");
      frame->flags |= 0x1;
      break;
    case 0x41:
      //printf("Got int15 wait on event\n");
      switch(frame->al) {
        case 0x00:{
          u32 unused ptr = (frame->es << 4) + frame->di;
          //printf("Waiting for event %x\n", ptr);
          //printf("Timeout: %d\n", frame->bx);
        } break;
      }
      break;
    default:
      printf("Got unknown int15 interrupt: 0x%x\n", frame->ax);
      frame->flags |= 0x1;
      break;
  }
}

void bios_init() {
  printf("Installing real mode interrupt handlers\n");
  itr_set_real_mode_handler(0x13, bios_handle_disk_interrupt, NULL);
  itr_set_real_mode_handler(0x16, bios_handle_keyboard_interrupt, NULL);
  itr_set_real_mode_handler(0x11, bios_handle_get_equipment_list, NULL);
  itr_set_real_mode_handler(0x12, bios_handle_memory_size_interrupt, NULL);
  itr_set_real_mode_handler(0x1a, bios_handle_get_system_time, NULL);
  itr_set_real_mode_handler(0x14, bios_handle_serial_port, NULL);
  itr_set_real_mode_handler(0x15, bios_handle_int15, NULL);
}

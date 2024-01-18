#include "output.h"
#include "postcode.h"
#include "util.h"

void printRamMsg(unsigned int ram, const char *msg) {
  printf("%d KB RAM %s\r", ram / 1024, msg);
}

unsigned int probeRam(unsigned int start, unsigned int stride,
                      int print_message) {
  unsigned int end = 0xffffffff;
  unsigned int bytes_found = 0;
  unsigned int i = 0;
  for (i = start + stride - 4; i < end; i += stride) {
    volatile unsigned int *test_point = (unsigned int *)i;
    int old_val = *test_point;
    *test_point = 0xAA55AA55;
    asm("wbinvd");
    if (*test_point == 0xAA55AA55) {
      *test_point = 0x55AA55AA;
      asm("wbinvd");
      if (*test_point == 0x55AA55AA) {
        bytes_found = (i + 4 - start);
        if (print_message && (bytes_found / stride) % 64 == 0 &&
            bytes_found > 0) {
          printRamMsg(bytes_found, "FOUND");
        }
      } else {
        break;
      }
    } else {
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
    volatile unsigned int *test_point = (unsigned int *)i;
    int old_val = *test_point;
    *test_point = 0xAA55AA55;
    asm("invd");
    if (*test_point == 0xAA55AA55) {
      *test_point = 0x55AA55AA;
      asm("invd");
      if (*test_point == 0x55AA55AA) {
        bytes += 4;
        if (print_message && bytes % 1024 == 0) {
          printRamMsg(bytes, "OK");
        }
      } else {
        printRamMsg(bytes, "Failed");
        break;
      }
    } else {
      printRamMsg(bytes, "Failed");
      break;
    }
    *test_point = old_val;
  }
  if (print_message && bytes == end - start) {
    printRamMsg(bytes, "OK");
  }
  return bytes;
}

unsigned int probeAll(unsigned int stride, unsigned int top,
                      int print_message) {
  unsigned int total = 0;
  for (unsigned int i = 0; i < top;) {
    unsigned int ram = probeRam(i, 1024, 0);
    if (ram > 0) {
      if (print_message) {
        printf("Found RAM region of size %d KB From 0x%x to 0x%x\n", ram / 1024,
               i, i + ram - 1);
      }
      i += (ram - (ram % stride));
    } else {
      i += stride;
    }
    total += ram;
  }
  return total;
}


void checkRam() {
  unsigned int lower = probeRam(0x0, 0x400, 1);
  unsigned int upper = probeRam(0x100000, 0x400, 1);
  printf("%d KB Conventional Memory\n", lower / 1024);
  printf("%d KB Upper Memory\n", upper / 1024);
  printf("%d KB Total\n", (lower + upper) / 1024);
  /*printf("Testing Conventional Memory\n");
  void(*testRamLower)(unsigned int, unsigned int, int) = (void*)shadowed_call(testRam);
  testRamLower(0x0, lower, 1);
  printf("\nTesting Upper Memory\n");
  testRamLower(0x100000, upper, 1);
  printf("\n");*/
}

int read_pit() {
  outb(0x43, 0x00);
  unsigned char l = inb(0x40);
  unsigned char h = inb(0x40);
  return (h << 8) | l;
}

#define NUM_SAMPLES 128

int probeSpeed() {
  asm("cli");
  int samples[NUM_SAMPLES];
  for (int i = 0; i < NUM_SAMPLES; i++) {
    int start = read_pit();
    for (int i = 0; i < 10000; i++) {
      int i = 12;
      i++;
      int j = i - 10;
      int result = i / j;
      asm("mov %0, %%eax" : "=r"(result));
    }
    int end = read_pit();
    if (end < start) {
      end += 0x10000;
    }
    samples[i] = (int)((end - start));
  }
  int total = 0;
  // TODO report variance etc
  for (int i = 0; i < NUM_SAMPLES; i++) {
    total += samples[i];
  }
  asm("sti");
  return total / NUM_SAMPLES;
}


int probeMemSpeed() {
  // DIsable cache
  asm("cli");
  asm("mov %cr0, %eax");
  asm("or %eax, 0x40000000");
  asm("mov %eax, %cr0");
  int samples[NUM_SAMPLES];
  for (int i = 0; i < NUM_SAMPLES; i++) {
    int start = read_pit();
    volatile unsigned int *test_point = (unsigned int *)0x200000;
    for (int i = 0; i < 1000; i++) {
      int original = *test_point;
      *test_point = 0xAA55AA55;
      *test_point = 0x55AA55AA;
      if (*test_point != 0x55AA55AA) {
        printf("Failed to write to memory\n");
        break;
      }
      *test_point = original;
      test_point++;
    }
    int end = read_pit();
    if (end < start) {
      end += 0x10000;
    }
    samples[i] = (int)((end - start));
  }
  int total = 0;
  // TODO report variance etc
  for (int i = 0; i < NUM_SAMPLES; i++) {
    total += samples[i];
  }
  //printf("Memory speed: %d\n", (int)((end - start)));
  // Enable cache
  asm(
      "mov %%cr0,  %%eax\n"
      "and %%eax, 0xBFFFFFFF\n"
      "wbinvd\n"
      "mov %%eax, %%cr0\n"
      : : : "eax");
  asm("sti");
  return total / NUM_SAMPLES;
}

void doPost() { checkRam(); }

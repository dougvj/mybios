#ifndef VGA_H
#define VGA_H
void vgaSetChar(int index, char c);

void vgaSetCharLc(int line, int col, char c);

void vgaCls();
void vgaPutChar(char c);

#endif

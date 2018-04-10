#define TXT_BASE 0xB8000



void vgaSetChar(int index, char c) {
    char* output = (char*)TXT_BASE + (index * 2);
    output[0] = c;
    output[1] = 0x1F;
}

void vgaSetCharLc(int line, int col, char c) {
    int index = line * 80 + col;
    vgaSetChar(index, c);
}

void vgaCls() {
    for (int i = 0; i < 80; i++) {
        for (int j = 0; j < 25; j++) {
            vgaSetCharLc(j, i, ' ');
        }
    }
}

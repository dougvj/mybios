#define TXT_BASE 0xB8000

static char cur_line = 0, cur_col = 0;

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
    int* output = (int*) TXT_BASE;
    for (int i = 0; i < 1000; i++) {
        output[i] = 0x1f201f20;
    }
}

void vgaScroll() {
    int* output = (int*) TXT_BASE;
    for (int line = 1; line < 25; line++) {
        for (int col = 0; col < 40; col++) {
            output[col + (line - 1) * 40] = output[col + line * 40];
        }
    }
    //Blank the last line
    for (int i = 0; i < 40; i++) {
        output[40 * 24 + i] = 0x1f201f20;
    }
}

void vgaPutChar(char c) {
    switch(c) {
        case '\n':
            cur_col = 0;
            cur_line++;
            break;
        case '\r':
            cur_col = 0;
            break;
        default:
            vgaSetCharLc(cur_line, cur_col, c);
            cur_col++;
            break;
    }
    if (cur_col >= 80) {
        cur_line++;
        cur_col = 0;
    }
    if (cur_line >= 25) {
        vgaScroll();
        cur_line = 24;
    }
}

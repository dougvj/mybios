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
    for (int i = 0; i <  80; i++) {
        for (int j = 0; j < 25; j++) {
            vgaSetCharLc(j, i, ' ');
        }
    }
}

void vgaScroll() {
    char* output = (char*) TXT_BASE;
    for (int line = 1; line < 25; line++) {
        for (int col = 0; col < 160; col++) {
            output[col + (line - 1) * 160] = output[col + line * 160];
        }
    }
    //Blank the last line
    for (int col = 0; col < 80; col++) {
        output[160 * 24 + col * 2] = ' ';
        output[160 * 24 + col * 2 + 1] = 0x1F;
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
    }
    if (cur_col == 80) {
        cur_line++;
        cur_col = 0;
    }
    if (cur_line == 25) {
        vgaScroll();
        cur_line = 24;
    }
}

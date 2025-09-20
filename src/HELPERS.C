#include "helpers.h"

static char screenbuf[80*25*2];

static void print_line(unsigned char *ptr,
		       unsigned char *asciiptr,
		       unsigned addr) {
    int j;
    unsigned char v;

    /* print address in hex format */
    printf("%04X | ", addr);

    /* print each byte in hex format */
    for(j=0; j<16; ++j) {
	printf("%02X ", *(ptr++));
    }

    /* show ASCII representation */
    printf(" | ");
    for(j=0; j<16; ++j) {
	v = *(asciiptr++);
	if(v >= 0x20 && v <= 0x7E) {
	    putchar((char)v);
	} else {
	    putchar('.');
	}
    }
    printf(" | \n");
}

void print_block(unsigned char buf[]) {
    int i,j;
    unsigned char *ptr = buf;
    unsigned char *asciiptr = buf;
    unsigned addr = 0;

    for(i=0; i<16; ++i) {
	print_line(ptr, asciiptr, addr);
	addr += 16;
	ptr += 16;
	asciiptr += 16;
    }
    printf("Press any key to continue to next 256 bytes.");
    getch();
    printf("\r");
    for(i=0; i<16; ++i) {
	print_line(ptr, asciiptr, addr);
	addr += 16;
	ptr += 16;
	asciiptr += 16;
    }
}

void set_rect_attr(int left, int top, int right, int bottom, unsigned char attr) {
    unsigned width = right - left + 1;
    unsigned height = bottom - top + 1;
    unsigned cells = width * height;
    unsigned i;

    void *buf = malloc(cells * 2);
    if(!buf) {
	return;
    }

    if(gettext(left, top, right, bottom, buf)) {
	unsigned char* p = (unsigned char*)buf;
	for(i=0; i<cells; ++i) {
	    p[2*i + 1] = attr;
	}
	puttext(left, top, right, bottom, buf);
    }
    free(buf);
}

void set_hl() {
    textbackground(WHITE);
    textcolor(BLACK);
}

void set_regular() {
    textbackground(BLACK);
    textcolor(LIGHTGRAY);
}

void store_screen() {
    gettext(1,1,80,25,screenbuf);
}

void restore_screen() {
    puttext(1,1,80,25,screenbuf);
}

void draw_textbox(int left, int top, int right, int bottom, const char *title) {
    window(left, top, right, bottom);
    clrscr();
    gotoxy(left, top);
    cprintf("%s", title);
}

int folder_exists(const char* path) {
    struct ffblk ff;
    if(findfirst(path, &ff, FA_DIREC) == 0) {
	return (ff.ff_attrib & FA_DIREC) != 0;
    }
    return 0;
}

int file_exists(const char* path) {
    struct ffblk ff;
    if(findfirst(path, &ff, FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_ARCH) == 0) {
	return (ff.ff_attrib & FA_DIREC) == 0;
    }
    return 0;
}

void disable_cursor() {
    union REGS r;
    r.h.ah = 1;
    r.h.ch = 0x20;
    r.h.cl = 0x20;
    int86(0x10, &r, &r);
}

void enable_cursor() {
    union REGS r;
    r.h.ah = 1;
    r.h.ch = 6;
    r.h.cl = 7;
    int86(0x10, &r, &r);
}

void build_dos_filename(const struct FAT32File* f, char *path) {
    char base[9];
    char ext[4];
    char* ptr;

    memset(path, 0, 13);
    memcpy(base, f->basename, 8);
    base[8] = 0;
    ptr = (char*)strchr(base, ' ');
    *ptr = 0;
    strcat(path, base);
    memcpy(ext, f->extension, 3);
    ext[3] = 0;
    ptr = (char*)strchr(ext, ' ');
    *ptr = 0;
    if(ptr != ext) {
	strcat(path, ".");
    }
    strcat(path, ext);
}
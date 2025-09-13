#include "view.h"

static int cursor_pos;

static void set_hl() {
    textbackground(WHITE);
    textcolor(BLACK);
}

static void set_regular() {
    textbackground(BLACK);
    textcolor(WHITE);
}

void view_print_files() {
    unsigned int i;
    const struct FAT32File* entry = 0;

    window(1,2,40,22);
    clrscr();

    for(i=0; i<fat32_nrfiles; ++i) {
	gotoxy(1,i+1);
	entry = &fat32_files[i];

	if(entry->attrib & MASK_DIR) {
	    cprintf("  %.11s  [DIR] %08X", entry->basename, entry->cluster);
	} else {
	    cprintf("  %.8s.%.3s %10ul bytes", entry->basename, entry->extension, entry->filesize);
	}

	if(i >= 20) {
	    break;
	}
    }

    window(1,1,80,24);
}

void view_init() {
    set_regular();
    clrscr(); /* clear complete screen */

    gotoxy(1,1);
    set_hl();
    clreol();
    cputs("SD-CARD EXPLORER 0.1");

    gotoxy(1,24);
    clreol();
    cputs("F1: HELP | F2: CONNECT | F10: EXIT");

    set_regular();
}

void view_print_volume_label(const char* lbl) {
    gotoxy(60, 24);
    set_hl();
    cprintf("MOUNT: %.11s", lbl);
    set_regular();
}

void view_close() {
    textbackground(BLACK);
    textcolor(WHITE);
    clrscr();
}

void view_set_cursor(unsigned char pos) {
    gotoxy(1, pos+2);
    putch('>');
    cursor_pos = pos;
}

void view_move_cursor(char d) {
    gotoxy(1, cursor_pos+2);
    putch(' ');
    cursor_pos += d;
    if(cursor_pos < 0) {
	cursor_pos = fat32_nrfiles;
    }
    if(cursor_pos >= fat32_nrfiles) {
	cursor_pos = 0;
    }

    gotoxy(1, cursor_pos+2);
    putch('>');
}

int view_get_cursor_pos() {
    return cursor_pos;
}
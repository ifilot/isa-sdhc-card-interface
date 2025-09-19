#include "view.h"

static int cursor_pos;
static int start_pos;

void view_set_cursor() {
    gotoxy(1, cursor_pos+2);
    putch('>');
    set_rect_attr(1,cursor_pos+2,40,cursor_pos+2,BLACKONWHITE);
}

void view_remove_cursor() {
    gotoxy(1,cursor_pos+2);
    putch(' ');
    set_rect_attr(1,cursor_pos+2,40,cursor_pos+2,WHITEONBLACK);
}

void view_print_files() {
    unsigned int i,ctr;
    window(1,2,40,23);
    clrscr();

    ctr = 0;
    for(i=start_pos; i<fat32_nrfiles; ++i) {
	view_print_entry(++ctr, i);

	/* print at most 22 entries */
	if(ctr >= 22) {
	    break;
	}
    }

    window(1,1,80,25);
}

void view_init() {
    set_regular();
    window(1,1,80,25);
    clrscr(); /* clear complete screen */

    gotoxy(1,1);
    set_hl();
    clreol();
    cputs("SD-CARD EXPLORER 0.1");

    view_display_commands();
}

void view_display_commands() {
    gotoxy(1,25);
    set_hl();
    clreol();
    cputs("F1: HELP | F3: COPY | F10: EXIT");
    set_regular();
}

void view_print_volume_label(const char* lbl) {
    set_hl();
    window(1,1,80,25);
    gotoxy(60,25);
    cprintf("SDCARD: %.11s", lbl);
    set_regular();
}

void view_close() {
    window(1,1,80,25);
    textbackground(BLACK);
    textcolor(WHITE);
    clrscr();
}

void view_reset_cursor() {
    cursor_pos = 0;
    start_pos = 0;
    view_set_cursor();
}

void view_move_cursor(int d) {
    int old_start_pos = start_pos;

    view_remove_cursor();

    /* set new cursor position */
    cursor_pos += d;

    if((cursor_pos + start_pos) >= (int)fat32_nrfiles) {
	if(fat32_nrfiles <= 22) {
	    cursor_pos = fat32_nrfiles - 1;
	    start_pos = 0;
	} else {
	    cursor_pos = 21;
	    start_pos = fat32_nrfiles - 22;
        }
    } else if((cursor_pos + start_pos) < 0) {
	cursor_pos = 0;
	start_pos = 0;
    } else {
	if(cursor_pos > 21) {
	    d = cursor_pos - 21;
	    start_pos += d;
	    cursor_pos -= d;
	} else if(cursor_pos < 0) {
	    start_pos += cursor_pos;
	    cursor_pos = 0;
	}
    }

    d = start_pos - old_start_pos;
    switch(d) {
	case 0:
	    /* do nothing */
	break;
	case -1:
	    view_scroll_up();
	break;
	case 1:
	    view_scroll_down();
	break;
	default:
	    view_print_files();
	break;
    }

    view_set_cursor();
}

int view_get_cursor_pos() {
    return start_pos + cursor_pos;
}

void view_scroll_down() {
    window(1,2,40,23);
    gotoxy(1,1);
    delline();
    view_print_entry(22, start_pos + 21);
    window(1,1,80,25);
}

void view_scroll_up() {
    window(1,2,40,23);
    gotoxy(1,1);
    insline();
    view_print_entry(0, start_pos);
    window(1,1,80,25);
}

void view_print_entry(unsigned char pos, unsigned int id) {
    const struct FAT32File *entry = &fat32_files[id];
    gotoxy(1,pos);
    if(entry->attrib & MASK_DIR) {
	cprintf("  %-25s [DIR]", entry->basename);
    } else {
	cprintf("  %.8s.%.3s           %8lu", entry->basename, entry->extension, entry->filesize);
    }
}
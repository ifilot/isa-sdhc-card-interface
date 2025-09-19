#include "nav.h"

static unsigned int nav_nrfiles;
static struct NavFile nav_files[NAV_MAX_FILES];
static int cursor_pos;
static int start_pos;

void nav_init() {
    nav_read_files();
    nav_print_files();
}

void nav_display_commands() {
    gotoxy(1,25);
    set_hl();
    clreol();
    cputs("F1: HELP | F2: MKDIR | F10: EXIT");
    set_regular();
}

void nav_change_folder(const struct NavFile* f) {
    if(f->attrib & NAV_MASK_DIR) {
	chdir(f->filename);
    }
}

const struct NavFile* nav_get_file_entry(unsigned id) {
    if(id < nav_nrfiles) {
	return &nav_files[id];
    }
}

void nav_read_files() {
    struct ffblk file;
    int done;
    unsigned int ctr = 0;

    done = findfirst("*.*", &file, FA_NORMAL | FA_DIREC);

    while(!done && ctr < NAV_MAX_FILES) {
	memcpy(nav_files[ctr].filename, file.ff_name, 13);
	nav_files[ctr].attrib = file.ff_attrib;
	nav_files[ctr].filesize = file.ff_fsize;
	done = findnext(&file);
	ctr++;
    }

    nav_nrfiles = ctr;
    nav_sort_files();
}

void nav_sort_files() {
    qsort(nav_files, nav_nrfiles, sizeof(struct NavFile), nav_file_compare);
}

int nav_file_compare(void* f1, void* f2) {
    const struct NavFile* file1 = (const struct NavFile*)f1;
    const struct NavFile* file2 = (const struct NavFile*)f2;

    unsigned char is_dir1 = (file1->attrib & NAV_MASK_DIR) != 0;
    unsigned char is_dir2 = (file2->attrib & NAV_MASK_DIR) != 0;

    if(is_dir1 && !is_dir2) {
	return -1;
    } else if(!is_dir1 && is_dir2) {
	return 1;
    }

    return strcmp(file1->filename, file2->filename);
}

void nav_print_files() {
    unsigned int i,ctr;
    window(41,2,80,23);
    clrscr();

    ctr = 0;
    for(i=start_pos; i<nav_nrfiles; ++i) {
	nav_print_entry(++ctr, i);

	/* print at most 22 entries */
	if(ctr >= 22) {
	    break;
	}
    }

    window(1,1,80,25);
}

void nav_reset_cursor() {
    cursor_pos = 0;
    start_pos = 0;
    nav_set_cursor();
}

void nav_remove_cursor() {
    gotoxy(41,cursor_pos+2);
    putch(' ');
    set_rect_attr(41,cursor_pos+2,80,cursor_pos+2,WHITEONBLACK);
}

void nav_set_cursor() {
    gotoxy(41, cursor_pos+2);
    putch('>');
    set_rect_attr(41,cursor_pos+2,80,cursor_pos+2,BLACKONWHITE);
}

void nav_move_cursor(int d) {
    int old_start_pos = start_pos;

    /* remove old cursor */
    nav_remove_cursor();

    /* set new cursor position */
    cursor_pos += d;

    if((cursor_pos + start_pos) >= (int)nav_nrfiles) {
	if(nav_nrfiles <= 22) {
	    cursor_pos = nav_nrfiles - 1;
	    start_pos = 0;
	} else {
	    cursor_pos = 21;
	    start_pos = nav_nrfiles - 22;
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
	    nav_scroll_up();
	break;
	case 1:
	    nav_scroll_down();
	break;
	default:
	    nav_print_files();
	break;
    }

    nav_set_cursor();
}

int nav_get_cursor_pos() {
    return start_pos + cursor_pos;
}

void nav_scroll_down() {
    window(41,2,80,23);
    gotoxy(1,1);
    delline();
    nav_print_entry(22, start_pos + 21);
    window(1,1,80,25);
}

void nav_scroll_up() {
    window(41,2,80,23);
    gotoxy(1,1);
    insline();
    nav_print_entry(0, start_pos);
    window(1,1,80,25);
}

void nav_print_entry(unsigned char pos, unsigned int id) {
    const struct NavFile *f = &nav_files[id];
    gotoxy(1,pos); /* function assumes to be in NAV window !!*/
    if(f->attrib & NAV_MASK_DIR) { /* check if folder */
	cprintf("  %-25s [DIR]", f->filename);
    } else {
	cprintf("  %-22s %8lu", f->filename, f->filesize);
    }
}

void nav_create_folder() {
    char c = 0;
    unsigned ctr = 0;
    char dbuf[9] = {0};

    store_screen();

    /* print window */
    draw_textbox(30,14,50,15,"Enter folder name:");
    gotoxy(1,2);

    /* put in writing mode */
    while(c != 0x0D) {
	c = getch();
	gotoxy(1+ctr, 2);
	if(ctr < 8) {
	    if((c>='0' & c<='9') || (c>='A' && c<='Z') || (c>='a' && c<='z')) {
		dbuf[ctr++] = c;
		putch(c);
	    }
	}
	if(ctr > 0 && c == 8) {
	    gotoxy(ctr, 2);
	    putch(' ');
	    dbuf[--ctr] = 0;
	    gotoxy(1+ctr, 2);
	}
	if(c == 0x1B) {
	    break;
	}
    }

    if(c == 0x0D) {
	mkdir(dbuf);
    }

    window(1,1,80,25);
    restore_screen();
    nav_read_files();
    nav_print_files();
    nav_reset_cursor();
}
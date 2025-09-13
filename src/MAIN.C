#include <stdio.h>
#include <dos.h>

#include "sd.h"
#include "fat32.h"
#include "helpers.h"
#include "view.h"

#define STATE_MAIN_MENU 0
#define STATE_SD        1
#define STATE_EXIT      -1

int state_main_menu();
int state_sd();

int main() {
    const struct FAT32File* f;
    int c;
    int ret;
    int state = STATE_MAIN_MENU;

    /* initialize view */
    view_init();

    while(state != STATE_EXIT) {
	switch(state) {
	    case STATE_MAIN_MENU:
		state = state_main_menu();
	    break;
	    case STATE_SD:
		state = state_sd();
	    break;
	    default:
		state = state_main_menu();
	    break;
	}
    }

    /* disable SD card */
    sddis(BASEPORT);

    /* close view */
    view_close();

    return 0;
}

int state_main_menu() {
    int c;
    while(1) {
	c = getch();
	if(c == 0) { /* special key */
	    c = getch();
	    switch(c) {
		case 0x44:
		    return STATE_EXIT;
		case 0x3C:
		    sd_boot();
		    fat32_open_partition();
		    view_print_volume_label(fat32_partition.volume_label);
		    fat32_read_current_folder();
		    view_print_files();
		    view_reset_cursor();
		    return STATE_SD;
	    }
	}
    }
}

int state_sd() {
    unsigned char c;
    int fpos;
    const struct FAT32File* f;

    while(1) {
	c = getch();
	if(c == 0) {
	    c = getch();
	    switch(c) {
		case 0x44:
		    return STATE_EXIT;
		case 0x49:
		    view_move_cursor(-22);
		break;
		case 0x51:
		    view_move_cursor(22);
		break;
	    }
	}
	switch(c) {
	    case 0x50:
		view_move_cursor(1);
	    break;
	    case 0x48:
		view_move_cursor(-1);
	    break;

	    case 0x0D:
		fpos = view_get_cursor_pos();
		f = fat32_get_file_entry(fpos);
		if(f->attrib & MASK_DIR) {
		    fat32_set_current_folder(f);
		    view_print_files();
		    view_reset_cursor();
		}
	    break;
	}
    }
}
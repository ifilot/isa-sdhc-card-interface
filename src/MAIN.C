#include <stdio.h>
#include <dos.h>

#include "sd.h"
#include "fat32.h"
#include "helpers.h"
#include "view.h"
#include "nav.h"

#define STATE_MAIN_MENU 0
#define STATE_SD        1
#define STATE_NAV       2
#define STATE_EXIT      -1
#define CWDBUFLEN	256

int state_main_menu();
int state_sd();
int state_nav();
void init_sd();

static char cwd[CWDBUFLEN];

int main() {
    const struct FAT32File* f;
    int c;
    int ret;
    int state = STATE_SD;

    /* store current working directory */
    getcwd(cwd, CWDBUFLEN);

    /* initialize view */
    view_init();

    /* initialize navigation */
    nav_init();

    /* initialize SD card */
    init_sd();

    while(state != STATE_EXIT) {
	switch(state) {
	    case STATE_MAIN_MENU:
		state = state_main_menu();
	    break;
	    case STATE_SD:
		state = state_sd();
	    break;
	    case STATE_NAV:
		state = state_nav();
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

    /* restore starting cwd */
    chdir(cwd);

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
	    case 0x09:
		view_remove_cursor();
		nav_set_cursor();
		nav_display_commands();
		return STATE_NAV;
	    case 0x3D:
		fpos = view_get_cursor_pos();
		f = fat32_get_file_entry(fpos);
		fat32_transfer_file(f);
		nav_read_files();
		nav_print_files();
		nav_reset_cursor();
	    break;
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
}

int state_nav() {
    unsigned char c;
    int fpos;
    const struct NavFile* f;

    while(1) {
	c = getch();
	if(c == 0) {
	    c = getch();
	    switch(c) {
		case 0x3C:
		    nav_create_folder();
		break;
		case 0x44:
		    return STATE_EXIT;
		case 0x49:
		    nav_move_cursor(-22);
		break;
		case 0x51:
		    nav_move_cursor(22);
		break;
	    }
	}
	switch(c) {
	    case 0x09:
		nav_remove_cursor();
		view_set_cursor();
		view_display_commands();
		return STATE_SD;
	    case 0x50:
		nav_move_cursor(1);
	    break;
	    case 0x48:
		nav_move_cursor(-1);
	    break;

	    case 0x0D:
		fpos = nav_get_cursor_pos();
		f = nav_get_file_entry(fpos);
		if(f->attrib & NAV_MASK_DIR) {
		    nav_change_folder(f);
		    nav_read_files();
		    nav_print_files();
		    nav_reset_cursor();
		}
	    break;
	}
    }
}

void init_sd() {
    sd_boot();
    fat32_open_partition();
    view_print_volume_label(fat32_partition.volume_label);
    fat32_read_current_folder();
    view_print_files();
    view_reset_cursor();
}
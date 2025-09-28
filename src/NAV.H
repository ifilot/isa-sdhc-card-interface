#ifndef _NAV_H
#define _NAV_H

#include <stdio.h>
#include <dir.h>
#include <string.h>
#include "helpers.h"

#define NAV_MAX_FILES 256

#define FA_NORMAL 	0x00
#define FA_DIREC  	0x10
#define NAV_MASK_DIR 	0x10

struct NavFile {
    char filename[13];
    unsigned char attrib;
    unsigned long filesize;
};

extern struct NavFile nav_files[NAV_MAX_FILES];
extern unsigned int nav_nrfiles;

void nav_init();

void nav_display_commands();

void nav_change_folder(const struct NavFile* f);

int nav_get_cursor_pos();

const struct NavFile* nav_get_file_entry();

void nav_read_files();

void nav_sort_files();

int nav_file_compare(void* f1, void* f2);

void nav_print_files();

void nav_remove_cursor();

void nav_set_cursor();

void nav_reset_cursor();

void nav_move_cursor(int d);

void nav_scroll_down();

void nav_scroll_up();

void nav_print_entry(unsigned char pos, unsigned int id);

void nav_create_folder();

#endif
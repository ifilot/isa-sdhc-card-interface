#ifndef _VIEW_H
#define _VIEW_H

#include <conio.h>
#include "fat32.h"
#include "helpers.h"
#include "version.h"

void view_init();

void view_display_commands();

void view_print_volume_label(const char* lbl);

void view_print_files();

void view_close();

void view_remove_cursor();

void view_set_cursor();

void view_reset_cursor();

void view_move_cursor(int d);

int view_get_cursor_pos();

void view_scroll_down();

void view_scroll_up();

void view_print_entry(unsigned char pos, unsigned int id);

#endif
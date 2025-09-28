#ifndef __HELPERS_H
#define __HELPERS_H

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <dir.h>
#include <dos.h>

#include "fatstruc.h"

#define BLACKONWHITE 112
#define WHITEONBLACK   7

/*
 * Print 512-block to the screen
 */
void print_block(unsigned char buf[]);

void set_rect_attr(int left, int top, int right, int bottom, unsigned char attr);

void set_regular();

void set_hl();

void store_screen();

void restore_screen();

void draw_textbox(int left, int top, int right, int bottom, const char* title);

int folder_exists(const char* path);

int file_exists(const char* path);

void disable_cursor();

void enable_cursor();

void build_dos_filename(const struct FAT32File* f, char* path);

#endif
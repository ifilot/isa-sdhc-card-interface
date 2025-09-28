#ifndef _FAT32_H
#define _FAT32_H

#include <stdio.h>
#include <time.h>

#include "fatstruc.h"
#include "sdcmds.h"

#define F32LLSZ  128  /* maximum linked list size */
#define F32MXFL  256  /* maximum number of files in folder to parse */
#define F32MXDIR 256  /* maximum number of folders that can be copied */
#define MAXPATH   80  /* maximum number of characters in path */

#define MASK_DIR (1 << 4)
#define FILE_ENTRY 0
#define FOLDER_ENTRY 1

extern struct FAT32Partition fat32_partition;
extern struct FAT32File fat32_files[F32MXFL];
extern unsigned int fat32_nrfiles;

void fat32_open_partition();

void fat32_print_partition_info();

void fat32_read_dir(struct FAT32Folder* folder, struct FAT32File buffer[]);

void fat32_read_current_folder();

unsigned long fat32_calculate_sector_address(unsigned long cluster,
					     unsigned char sector);

void fat32_build_linked_list(unsigned long nextcluster);

void fat32_sort_files();

void fat32_list_dir();

void fat32_set_current_folder(const struct FAT32File* entry);

const struct FAT32File* fat32_get_file_entry(unsigned int id);

int fat32_file_compare(const void* item1, const void* item2);

void fat32_read_sector(unsigned long);

unsigned long fat32_grab_cluster_address_from_fileblock(unsigned char* loc);

int fat32_transfer_file(const struct FAT32File* f, const char* path);

void fat32_transfer_folder(const struct FAT32File* f);

void fat32_build_path(struct FAT32Folder folders[], unsigned id, char path[]);

void fat32_transfer_files_in_folder(struct FAT32Folder* f, const char* basepath);

#endif
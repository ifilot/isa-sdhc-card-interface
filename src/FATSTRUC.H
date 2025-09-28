#ifndef _PARTITION_H
#define _PARTITION_H

struct FAT32Partition {
    unsigned bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned reserved_sectors;
    unsigned char number_of_fats;
    unsigned long sectors_per_fat;
    unsigned long root_dir_first_cluster;
    unsigned long fat_begin_lba;
    unsigned long lba_addr_root_dir;
    unsigned long sector_begin_lba;
    char volume_label[11];
};

struct FAT32Folder {
    char name[11];
    unsigned long cluster;
    unsigned nrfiles;
    unsigned char attrib;
    int reference;
};

struct FAT32File {
    char basename[8];
    char extension[3];
    unsigned char termbyte;
    unsigned char attrib;
    unsigned long cluster;
    unsigned long filesize;
};

#endif
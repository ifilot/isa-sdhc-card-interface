#include "fat32.h"

static unsigned char sdbuf[514];
static unsigned long fat32_linked_list[F32LLSZ];
unsigned int fat32_nrfiles;

struct FAT32Partition fat32_partition;
struct FAT32Folder fat32_root_folder;
struct FAT32Folder fat32_current_folder;
struct FAT32File fat32_files[F32MXFL];

void fat32_open_partition() {
    unsigned long lba = 0x00000000;
    char partname[12];

    /* read boot sector */
    cmd17(BASEPORT, 0x00000000, sdbuf);
    /*print_block(sdbuf);*/

    /* check for magic bytes */
    if(sdbuf[510] != 0x55 || sdbuf[511] != 0xAA) {
	printf("Could not read SD-card. Try to reinsert.\n");
	sddis(BASEPORT);
	return;
    }

    /* grab start address of first partition */
    lba = *(unsigned long*)(sdbuf + 0x1C6);
    /*printf("LBA: %08X\n", lba);*/

    /* read the partition information */
    fat32_read_sector(lba);

    /* store partition information */
    fat32_partition.bytes_per_sector = *(unsigned*)(sdbuf + 0x0B);
    fat32_partition.sectors_per_cluster = sdbuf[0x0D];
    fat32_partition.reserved_sectors = *(unsigned*)(sdbuf + 0x0E);
    fat32_partition.number_of_fats = sdbuf[0x10];
    fat32_partition.sectors_per_fat = *(unsigned long*)(sdbuf + 0x24);
    fat32_partition.root_dir_first_cluster = *(unsigned long*)(sdbuf + 0x2C);
    fat32_partition.fat_begin_lba = lba + fat32_partition.reserved_sectors;
    fat32_partition.sector_begin_lba = fat32_partition.fat_begin_lba +
	(fat32_partition.number_of_fats * fat32_partition.sectors_per_fat);
    fat32_partition.lba_addr_root_dir = fat32_calculate_sector_address(fat32_partition.root_dir_first_cluster, 0);

    /* grab information from root folder */
    cmd17(BASEPORT, fat32_partition.lba_addr_root_dir, sdbuf);
    memcpy(fat32_partition.volume_label, sdbuf, 11);
    /*printf("Partition name: %11s\n", fat32_partition.volume_label); */

    /* set both current folder and root folder */
    fat32_current_folder.cluster = fat32_partition.root_dir_first_cluster;
    fat32_current_folder.nrfiles = 0;
    memset(fat32_current_folder.name, 0x00, 11);
    fat32_root_folder = fat32_current_folder;
}

void fat32_print_partition_info() {
    printf("Bytes per sector: %i\n", fat32_partition.bytes_per_sector);
    printf("Sectors per cluster: %i\n", fat32_partition.sectors_per_cluster);
    printf("Reserved sectors: %i\n", fat32_partition.reserved_sectors);
    printf("Number of FATs: %i\n", fat32_partition.number_of_fats);
    printf("Root directory first cluster: %08X\n", fat32_partition.root_dir_first_cluster);
    printf("FAT begin LBA: %08X\n", fat32_partition.fat_begin_lba);
    printf("Root directory sector: %08X\n", fat32_partition.lba_addr_root_dir);
}

void fat32_read_dir(struct FAT32Folder* folder) {
    unsigned char ctr = 0;
    unsigned fctr = 0;
    unsigned i,j;
    unsigned char* locptr = 0;
    struct FAT32File *file = 0;
    unsigned long caddr = 0;

    /* build linked list */
    fat32_build_linked_list(folder->cluster);

    while(fat32_linked_list[ctr] != 0xFFFFFFFF && ctr < F32LLSZ) {
	caddr = fat32_calculate_sector_address(fat32_linked_list[ctr], 0);

	for(i=0; i<fat32_partition.sectors_per_cluster; ++i) {
	    fat32_read_sector(caddr);           /* read sector in memory */
	    locptr = (unsigned char*)(sdbuf);	/* set pointer to sector data */
	    for(j=0; j<16; ++j) { /* consume 16 files entries per sector */
		/* continue if an unused entry is encountered */
		if(*locptr == 0xE5) {
		    locptr += 32;
		    continue;
		}

		/* early exit is a zero is read */
		if(*locptr == 0x00) {
		    file = &fat32_files[fctr];
		    folder->nrfiles = fctr;
		    return;
		}

		/* check if we are reading a file or a folder, if so, add it */
		if((*(locptr + 0x0B) & 0x0F) == 0x00) {
		    file = &fat32_files[fctr];
		    memcpy(file->basename, locptr, 11); /* file name */
		    file->termbyte = 0;	/* by definition */
		    file->attrib = *(locptr + 0x0B);
		    file->cluster = fat32_grab_cluster_address_from_fileblock(locptr);
		    file->filesize = *(unsigned long*)(locptr + 0x1C);
		    fctr++;
		    /* printf("%i: %s %u bytes\n", fctr, file->basename, file->filesize); */

		    /* throw error message if we exceed buffer size */
		    if(fctr > F32MXFL) {
			printf("ERROR: Too many files in folder; cannot parse.");
			return;
		    }
		}
		locptr += 32; /* next file entry */
	    }
	    caddr++; /* next sector */
	}
	ctr++; /* next cluster */
    }
    folder->nrfiles = fctr;
}

void fat32_read_current_folder() {
    fat32_read_dir(&fat32_current_folder);
    fat32_sort_files();
    fat32_nrfiles = fat32_current_folder.nrfiles;
}

void fat32_set_current_folder(const struct FAT32File* entry) {
    if(entry->cluster == 0x00000000) { /* check for ROOT */
	fat32_current_folder = fat32_root_folder;
    } else {
	fat32_current_folder.cluster = entry->cluster;
	fat32_current_folder.nrfiles = 0;
	memcpy(fat32_current_folder.name, entry->basename, 11);
    }
    fat32_read_current_folder();
}

const struct FAT32File* fat32_get_file_entry(unsigned int id) {
    if(id < fat32_current_folder.nrfiles) {
	return &fat32_files[id];
    } else {
	return 0;
    }
}

unsigned long fat32_calculate_sector_address(unsigned long cluster,
					     unsigned char sector) {
    return fat32_partition.sector_begin_lba + (cluster - 2) *
	   fat32_partition.sectors_per_cluster + sector;
}

void fat32_build_linked_list(unsigned long nextcluster) {
    unsigned ctr = 0;
    unsigned item = 0;

    /* clear previous linked list */
    memset(fat32_linked_list, 0xFF, F32LLSZ * sizeof(unsigned long));

    while(nextcluster < 0x0FFFFFF8 && nextcluster != 0 && ctr < F32LLSZ) {
	fat32_linked_list[ctr] = nextcluster;
	fat32_read_sector(fat32_partition.fat_begin_lba + (nextcluster >> 7));
	item = nextcluster & 0x7F;
	nextcluster = *(unsigned long*)(sdbuf + item * 4);
	ctr++;
    }
}

void fat32_sort_files() {
    qsort(fat32_files, fat32_current_folder.nrfiles, sizeof(struct FAT32File), fat32_file_compare);
}

void fat32_list_dir() {
    unsigned int i=0;
    struct FAT32File *file = fat32_files;
    for(i=0; i<fat32_current_folder.nrfiles; ++i) {
	if(file->attrib & MASK_DIR) {
	    printf("%.8s %.3s %08lX DIR\n", file->basename, file->extension, file->cluster);
	} else {
	    printf("%.8s %.3s %08lX %lu\n", file->basename, file->extension, file->cluster, file->filesize);
	}
	file++;
    }
}

int fat32_file_compare(const void* item1, const void* item2) {
    const struct FAT32File *file1 = (const struct FAT32File*)item1;
    const struct FAT32File *file2 = (const struct FAT32File*)item2;

    unsigned char is_dir1 = (file1->attrib & MASK_DIR) != 0;
    unsigned char is_dir2 = (file2->attrib & MASK_DIR) != 0;

    if(is_dir1 && !is_dir2) {
	return -1;
    } else if(!is_dir1 && is_dir2) {
	return 1;
    }

    return strcmp(file1->basename, file2->basename);
}

void fat32_read_sector(unsigned long addr) {
    cmd17(BASEPORT, addr, sdbuf);
}

unsigned long fat32_grab_cluster_address_from_fileblock(unsigned char* loc) {
    return ((unsigned long)*(unsigned*)(loc + 0x14)) << 16 |
			   *(unsigned*)(loc + 0x1A);
}

void fat32_transfer_file(const struct FAT32File *f) {
    unsigned long caddr = 0;
    unsigned char ctr = 0;
    unsigned long bcnt = 0;
    int i;
    FILE *outfile;

    if(f->attrib & MASK_DIR) {
	return;
    }

    outfile = fopen(f->basename, "wb");

    fat32_build_linked_list(f->cluster);
    while(fat32_linked_list[ctr] != 0xFFFFFFFF && ctr < F32LLSZ && bcnt < f->filesize) {
	caddr = fat32_calculate_sector_address(fat32_linked_list[ctr], 0);

	for(i=0; i<fat32_partition.sectors_per_cluster; ++i) {
	    fat32_read_sector(caddr);

	    if((f->filesize - bcnt) > 512) {
		fwrite(sdbuf, sizeof(char), 512, outfile);
	    } else {
		fwrite(sdbuf, sizeof(char), f->filesize - bcnt, outfile);
		break;
	    }

	    bcnt += 512;
	}

	ctr++;
    }

    fclose(outfile);
}
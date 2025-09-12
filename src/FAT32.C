#include "fat32.h"

static unsigned char sdbuf[514];
static unsigned long fat32_linked_list[F32LLSZ];

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
    printf("LBA: %08X\n", lba);

    /* read the partition information */
    cmd17(BASEPORT, lba, sdbuf);

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
    printf("Partition name: %11s\n", fat32_partition.volume_label);

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
		    printf("%i: %s %u bytes\n", fctr, file->basename, file->filesize);

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
	cmd17(BASEPORT, fat32_partition.fat_begin_lba + (nextcluster >> 7), sdbuf);
	item = nextcluster & 0x7F;
	nextcluster = *(unsigned long*)(sdbuf + item * 4);
	ctr++;
    }
}

void fat32_read_sector(unsigned long addr) {
    cmd17(BASEPORT, addr, sdbuf);
}

unsigned long fat32_grab_cluster_address_from_fileblock(unsigned char* loc) {
    return ((unsigned long)*(unsigned*)(loc + 0x14)) << 16 |
			   *(unsigned*)(loc + 0x1A);
}
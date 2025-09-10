#include "fat32.h"

static unsigned char sdbuf[514];

void fat32_read_partition(struct Partition* partition) {
    unsigned long lba = 0x00000000;

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
    partition->bytes_per_sector = *(unsigned*)(sdbuf + 0x0B);
    partition->sectors_per_cluster = sdbuf[0x0D];
    partition->reserved_sectors = *(unsigned*)(sdbuf + 0x0E);
    partition->number_of_fats = sdbuf[0x10];
    partition->sectors_per_fat = *(unsigned long*)(sdbuf + 0x24);
    partition->root_dir_first_cluster = *(unsigned long*)(sdbuf + 0x2C);
    partition->fat_begin_lba = lba + partition->reserved_sectors;
    partition->sector_begin_lba = partition->fat_begin_lba +
	(partition->number_of_fats * partition->sectors_per_fat);
    partition->lba_addr_root_dir = fat32_calculate_sector_address(partition,
	partition->root_dir_first_cluster, 0);

    fat32_print_partition_info(partition);
    cmd17(BASEPORT, partition->lba_addr_root_dir, sdbuf);
    /*print_block(sdbuf);*/
}

void fat32_print_partition_info(const struct Partition *partition) {
    printf("Bytes per sector: %i\n", partition->bytes_per_sector);
    printf("Sectors per cluster: %i\n", partition->sectors_per_cluster);
    printf("Reserved sectors: %i\n", partition->reserved_sectors);
    printf("Number of FATs: %i\n", partition->number_of_fats);
    printf("Root directory first cluster: %08X\n", partition->root_dir_first_cluster);
    printf("FAT begin LBA: %08X\n", partition->fat_begin_lba);
    printf("Root directory sector: %08X\n", partition->lba_addr_root_dir);
}

unsigned long fat32_calculate_sector_address(struct Partition *partition,
					     unsigned long cluster,
					     unsigned char sector) {
    return partition->sector_begin_lba + (cluster - 2) *
	   partition->sectors_per_cluster + sector;
}
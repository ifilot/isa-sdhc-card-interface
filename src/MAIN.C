#include <conio.h>
#include <stdio.h>
#include <dos.h>

#include "sd.h"
#include "fat32.h"
#include "helpers.h"

static struct Partition partition;

int main() {
    sd_boot();
    fat32_read_partition(&partition);
    sddis(BASEPORT);

    return 0;
}
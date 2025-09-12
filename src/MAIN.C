#include <conio.h>
#include <stdio.h>
#include <dos.h>

#include "sd.h"
#include "fat32.h"
#include "helpers.h"

int main() {
    sd_boot();
    fat32_open_partition();
    fat32_read_current_folder();
    sddis(BASEPORT);

    return 0;
}
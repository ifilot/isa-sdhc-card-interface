#include <conio.h>
#include <stdio.h>
#include <dos.h>

#include "sdcmds.h"
#include "helpers.h"

/* DEFINE I/O PORTS */
#define BASEPORT  0x330

static unsigned char buffer[514];

int main() {
    int i, j, ctr;
    unsigned char v, c1, c2;
    unsigned char *ptr;

    clrscr();
    printf("SDCARD interface program\n");
    set_miso_high(BASEPORT);

    ctr = 0;
    v = 0xFF;
    while(v == 0xFF & ctr < 100) {
	sddis(BASEPORT);
	cmdclr(BASEPORT);
	sden(BASEPORT);
	v = cmd00(BASEPORT);	/* put card in idle state */
	ctr++;
    }
    if(ctr == 100) {
	printf("Cannot put card in idle mode. Exiting.");
	sddis(BASEPORT);
	return 1;
    }
    printf("CMD00 response: %02X\n", v);
    v = cmd08(BASEPORT);	/* send cmd08 */
    printf("CMD08 response: %02X ", v);

    /* retrieve remaining bytes of rsp5 */
    for(i=0; i<4; ++i) {
	v = sdrecvf(BASEPORT);
	printf("%02X ", v);
    }
    printf("\n");

    /* TRY ACMD41 */
    v = 0xFF;
    ctr = 0;
    while(v != 0x00 && ctr < 100) {
	v = cmd55(BASEPORT);
	if(v == 0xFF) {
	    break;	/* terminate attempts */
	}
	v = acmd41(BASEPORT);
	ctr++;
    }
    if(ctr == 100) {
	printf("Unable to send ACMD41 after %i attempts.", ctr);
    } else {
	printf("ACMD41 response: %02X (%i)\n", v, ctr);
    }

    /* CMD58 - READ OCR */
    v = cmd58(BASEPORT);
    printf("CMD58 response: %02X ", v);
    for(i=0; i<4; ++i) {
	v = sdrecvf(BASEPORT);
	printf("%02X ", v);
    }
    printf("\n");

    /* read boot sector */
    v = cmd17(BASEPORT, 0x0000, &buffer[0]);
    if(v != 0x00) {
	printf("Invalid CMD17 response: %02X\n", v);
	sddis(BASEPORT);
	return 1;
    } else {
	printf("CMD17 response: %02X\n", v);
    }
    getch();

    print_block(buffer);

    /* Test whether last two bytes of boot block are 0x55AA */
    if(buffer[510] == 0x55 && buffer[511] == 0xAA) {
	printf("Magic word identified: 0x55AA\n");
    } else {
	printf("Last two bytes do *not* match 0x55AA\n");
    }

    sddis(BASEPORT);	/* disable SD card */
    return 0;
}